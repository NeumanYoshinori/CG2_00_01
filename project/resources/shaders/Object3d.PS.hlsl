#include "Object3d.hlsli"

struct Material {
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
    float32_t environmentCoefficient;
};

static const int kNumDirectionalLight = 1;
static const int kNumPointLight = 2;
static const int kNumSpotLight = 1;

struct DirectionalLight {
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct PointLight {
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float radius;
    float decay;
    int32_t isActive;
};

struct SpotLight {
    float32_t4 color;
    float32_t3 position;
    float32_t intensity;
    float32_t3 direction;
    float32_t distance;
    float32_t decay;
    float32_t cosAngle;
    float32_t cosFalloffStart;
    int32_t isActive;
};

struct Camera {
    float32_t3 worldPosition;
};

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
cbuffer lightGroup : register(b1) {
    DirectionalLight directionalLights[kNumDirectionalLight];
    PointLight pointLights[kNumPointLight];
    SpotLight spotLights[kNumSpotLight];
}
ConstantBuffer<Camera> gCamera : register(b2);
TextureCube<float32_t4> gEnvironmentTexture : register(t1);

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    //  textureのα値が0.5以下のときにPixelを棄却
    if (textureColor.a <= 0.5) {
        discard;
    }
    
    // textureのα値が0のときにPixelを棄却
    if (textureColor.a == 0.0) {
        discard;
    }
    
    // output.colorの値が0の時にPixelを棄却
    if (output.color.a == 0.0) {
        discard;
    }
    
    if (gMaterial.enableLighting != 0) {
        float32_t3 color = { 0.0f, 0.0f, 0.0f };
        
        float32_t3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
        float32_t3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
        float32_t4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
        
        color += environmentColor.rgb * gMaterial.environmentCoefficient;
        
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        
        for (int i = 0; i < kNumDirectionalLight; i++) {
            float32_t3 halfVector = normalize(-directionalLights[i].direction + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess);
            float NdotL = dot(normalize(input.normal), -directionalLights[i].direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);

            // 拡散反射
            float32_t3 diffuse =
            gMaterial.color.rgb * textureColor.rgb * directionalLights[i].color.rgb * cos * directionalLights[i].intensity;
            // 鏡面反射
            float32_t3 specular =
            directionalLights[i].color.rgb * directionalLights[i].intensity * specularPow * directionalLights[i].color.rgb;
            // 拡散・鏡面反射
            color += diffuse + specular;
        }
        
        for (int j = 0; j < kNumPointLight; j++) {
            if (pointLights[j].isActive != 0) {
                float32_t3 pointLightDirection = normalize(input.worldPosition - pointLights[j].position);
                float32_t distance = length(pointLights[j].position - input.worldPosition);
                float32_t factor = pow(saturate(-distance / pointLights[j].radius + 1.0), pointLights[j].decay);
            
                float32_t3 halfVector = normalize(-pointLightDirection + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                float specularPow = pow(saturate(NDotH), gMaterial.shininess);
                float NdotL = dot(normalize(input.normal), -pointLightDirection);
                float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            
                // 拡散反射
                float32_t3 diffuse =
                gMaterial.color.rgb * textureColor.rgb * pointLights[j].color.rgb * cos * pointLights[j].intensity * factor;
                // 鏡面反射
                float32_t3 specular =
                pointLights[j].color.rgb * pointLights[j].intensity * factor * specularPow * pointLights[j].color.rgb;
                // 拡散・鏡面反射
                color += diffuse + specular;
            }
        }
        
        for (int k = 0; k < kNumSpotLight; k++) {
            if (spotLights[k].isActive != 0) {
                float32_t3 spotLightDirectionOnSurface = normalize(input.worldPosition - spotLights[k].position);
                float32_t distance = length(spotLights[k].position - input.worldPosition);
                float32_t attenuationFactor = pow(saturate(-distance / spotLights[k].distance + 1.0), spotLights[k].decay);
                float32_t cosAngle = dot(spotLightDirectionOnSurface, spotLights[k].direction);
                float32_t falloffFactor = saturate((cosAngle - spotLights[k].cosAngle) / (spotLights[k].cosFalloffStart - spotLights[k].cosAngle));
            
                float32_t3 halfVector = normalize(-spotLightDirectionOnSurface + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                float specularPow = pow(saturate(NDotH), gMaterial.shininess);
                float NdotL = dot(normalize(input.normal), -spotLightDirectionOnSurface);
                float cosS = pow(NdotL * 0.5f + 0.5f, 2.0f);
            
                // 拡散反射
                float32_t3 diffuse =
                gMaterial.color.rgb * textureColor.rgb * spotLights[k].color.rgb * cosS * spotLights[k].intensity * attenuationFactor * falloffFactor;
                // 鏡面反射
                float32_t3 specular =
                spotLights[k].color.rgb * spotLights[k].intensity * attenuationFactor * falloffFactor * specularPow * spotLights[k].color.rgb;
                // 拡散・鏡面反射
                color += diffuse + specular;
            }
        }
        
        output.color.rgb = color;
        
        // アルファは今まで通り
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}