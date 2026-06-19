#include "Particle.hlsli"

struct ParticleForGPU {
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4 color;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID) {
    VertexShaderOutput output;
    output.position = mul(input.position, gParticle[instanceId].WVP);
    float32_t2 texcoord = input.texcoord;
    texcoord.y = 1.0f - texcoord.y; // flip v
    output.texcoord = texcoord;
    output.color = gParticle[instanceId].color;
    return output;
}