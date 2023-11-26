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
    float2 boundingBoxTopAndBottom;
    float2 boundingBoxFrontAndBack;
    float2 boudningBoxSides;
};

groupshared Particle cache[numParticles];
 
[numthreads(1, numParticles, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{

    particleInput[groupThreadID.y].velocity.xy += float2(0.0f, 1.0f) * gravity * deltaTime;
    particleInput[groupThreadID.y].currentPosition.xy += particleInput[groupThreadID.y].velocity.xy * deltaTime;
    particleOutput[dispatchThreadID.y] = particleInput[groupThreadID.y];
    

}