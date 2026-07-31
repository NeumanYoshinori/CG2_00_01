#include "Object3d.hlsli"

struct TransformationMatrix {
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
    int32_t flipX;
    int32_t flipY;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    float32_t2 texcoord = input.texcoord;
    if (gTransformationMatrix.flipX != 0) {
        texcoord.x = 1.0f - texcoord.x; // flip v
    }
    if (gTransformationMatrix.flipY != 0) {
        texcoord.y = 1.0f - texcoord.y; // flip v
    }
    output.texcoord = texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.WorldInverseTranspose));
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    return output;
}