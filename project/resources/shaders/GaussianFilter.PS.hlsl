#include "FullScreen.hlsli"

struct Material {
    float32_t3 color;
    bool useSepia;
    float32_t scale;
    float32_t power;
    int kKernelSize;
    float32_t sigma;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

static const float32_t2 kIndex3x3[3][3] = {
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

static const float32_t2 kIndex5x5[5][5] = {
    { { -2.0f, -2.0f }, { -1.0f, -2.0f }, { 0.0f, -2.0f }, { 1.0f, -2.0f }, { 2.0f, -2.0f } },
    { { -2.0f, -1.0f }, { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f }, { 2.0f, -1.0f } },
    { { -2.0f, 0.0f }, { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 2.0f, 0.0f } },
    { { -2.0f, 1.0f }, { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 2.0f, 1.0f } },
    { { -2.0f, 2.0f }, { -1.0f, 2.0f }, { 0.0f, 2.0f }, { 1.0f, 2.0f }, { 2.0f, 2.0f } },
};

static const float32_t PI = 3.14159265f;

float gauss(float x, float y, float sigma) {
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

PixelShaderOutput main(VertexShaderOutput input) {
    uint32_t width, height; // 1. uvStepSizeの算出
    gTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
    
    // kernelを求める。weightは後で使う
    float32_t weight = 0.0f;
    float32_t kernel3x3[3][3];
    float32_t kernel5x5[5][5];
    
    if (gMaterial.kKernelSize == 3) {
        for (int32_t x = 0; x < 3; ++x) {
            for (int32_t y = 0; y < 3; ++y) {
                kernel3x3[x][y] = gauss(kIndex3x3[x][y].x, kIndex3x3[x][y].y, gMaterial.sigma);
                weight += kernel3x3[x][y];
            }
        }
    }
    else if (gMaterial.kKernelSize == 5) {
        for (int32_t x = 0; x < 5; ++x) {
            for (int32_t y = 0; y < 5; ++y) {
                kernel5x5[x][y] = gauss(kIndex5x5[x][y].x, kIndex5x5[x][y].y, gMaterial.sigma);
                weight += kernel5x5[x][y];
            }
        }
    }
    
    PixelShaderOutput output;
    output.color.rgb = float32_t3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    if (gMaterial.kKernelSize == 3) {
        for (int32_t i = 0; i < 3; ++i) { // 2. 3x3ループ
            for (int32_t j = 0; j < 3; ++j) {
                // 3. 現在のtexcoordを算出
                float32_t2 texcoord = input.texcoord + kIndex3x3[i][j] * uvStepSize;
                // 4. 色に1/9掛けて足す
                float32_t3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
                output.color.rgb += fetchColor * kernel3x3[i][j];
            }
        }
        
        output.color.rgb *= rcp(weight);
    }
    else if (gMaterial.kKernelSize == 5) {
        for (int32_t i = 0; i < 5; ++i) { // 2. 5x5ループ
            for (int32_t j = 0; j < 5; ++j) {
                // 3. 現在のtexcoordを算出
                float32_t2 texcoord = input.texcoord + kIndex5x5[i][j] * uvStepSize;
                // 4. 色に1/25掛けて足す
                float32_t3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
                output.color.rgb += fetchColor * kernel5x5[i][j];
            }
        }
        
        output.color.rgb *= rcp(weight);
    }
    else {
        output.color = gTexture.Sample(gSampler, input.texcoord);
    }
    
    return output;
}