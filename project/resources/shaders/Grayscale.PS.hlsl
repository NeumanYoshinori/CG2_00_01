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

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    float32_t value = dot(output.color.rgb, float32_t3(0.2125f, 0.7154f, 0.0721f));
    if (gMaterial.useSepia) {
        output.color.rgb = value * float32_t3(1.0f, 74.0f / 107.0f, 43.0f / 107.0f);
    }
    else {
        output.color.rgb = value * gMaterial.color;
    }
    return output;
}