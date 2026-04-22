#include "Object3d.hlsli"

struct Material {
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
};

struct DirectionalLight {
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct Camera {
    float32_t3 worldPosition;
};

static const int kNumPointLight = 2;

struct PointLight {
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float radius;
    float decay;
};

cbuffer lightGroup : register(b3) {
    PointLight pointLights[kNumPointLight];
}

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

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
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float32_t3 halfVectorD = normalize(-gDirectionalLight.direction + toEye);
        float NDotHD = dot(normalize(input.normal), halfVectorD);
        float specularPowD = pow(saturate(NDotHD), gMaterial.shininess);
        float NdotLD = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cosD = pow(NdotLD * 0.5f + 0.5f, 2.0f);
        float32_t3 color = { 0.0f, 0.0f, 0.0f };

        // 拡散反射
        float32_t3 diffuseDirectionalLight =
        gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cosD * gDirectionalLight.intensity;
        // 鏡面反射
        float32_t3 specularDirectionalLight =
        gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPowD * gDirectionalLight.color.rgb;
         // 拡散・鏡面反射
        color += diffuseDirectionalLight + specularDirectionalLight;
        
        for (int i = 0; i < kNumPointLight; i++) {
            float32_t3 pointLightDirection = normalize(pointLights[i].position - input.worldPosition);
            float32_t distance = length(pointLights[i].position - input.worldPosition);
            float32_t factor = pow(saturate(-distance / pointLights[i].radius + 1.0), pointLights[i].decay);
            
            float32_t3 halfVectorP = normalize(-pointLightDirection + toEye);
            float NDotHP = dot(normalize(input.normal), halfVectorP);
            float specularPowP = pow(saturate(NDotHP), gMaterial.shininess);
            float NdotLP = dot(normalize(input.normal), -pointLightDirection);
            float cosP = pow(NdotLP * 0.5f + 0.5f, 2.0f);
            
             // 拡散反射
            float32_t3 diffuse =
            gMaterial.color.rgb * textureColor.rgb * pointLights[i].color.rgb * cosP * pointLights[i].intensity * factor;
            // 鏡面反射
            float32_t3 specular =
            pointLights[i].color.rgb * pointLights[i].intensity * factor * specularPowP * pointLights[i].color.rgb;
            // 拡散・鏡面反射
            color += diffuse + specular;
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