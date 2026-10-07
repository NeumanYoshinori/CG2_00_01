#include "Line.hlsli"

struct Material {
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
    output.color = gMaterial.color;
    
    // output.colorの値が0の時にPixelを棄却
    if (output.color.a == 0.0) {
        discard;
    }
    
    return output;
}