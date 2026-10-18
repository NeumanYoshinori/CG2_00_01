#include "FullScreen.hlsli"

struct Material {
    float32_t4 color;
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
    output.color.rgb = gTexture.Sample(gSampler, input.texcoord).rgb * gMaterial.color.rgb;
    output.color.a = gTexture.Sample(gSampler, input.texcoord).a;
    return output;
}