//compute shader used for SPH simulation

struct Particle
{
    int size;
    float3 padding;
    float3 startPosition;
    float density;
    float3 currentPosition;
    float mass;
    float3 velocity;
    float bounceDampingFactor;
};

StructuredBuffer<Particle> particleInput : register(t0);
RWStructuredBuffer<Particle> particleOutput : register(u0); //Data we pass to and from the compute shader

cbuffer cb_simConstants : register(b0)
{
    float bounceDampingFactor;
    float gravity;
    float numParticles;
    float restDensity;
    float deltaTime;
    float3 padding;
};

[numthreads(1, 1, 1)]
void main(uint3 groupThreadID : SV_DispatchThreadID, int3 dispatchThreadID : SV_DispatchThreadID, int ID :SV_GroupIndex)
{
    for (int i = 0; i < numParticles;i++)
    {
        particleInput[i].velocity.xy += float2(0.0f, 1.0f) * gravity * deltaTime;
        particleInput[i].currentPosition.xy += particleInput[i].velocity.xy * deltaTime;
        particleOutput[i] = particleInput[i];
    }

}