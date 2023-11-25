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

RWStructuredBuffer<Particle> particleOutput : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> particleInput : register(t0);


//CREATE CONSTANT BUFFER IN .CPP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
cbuffer cb_simConstants : register(b0)
{
    int numParticles;
    float restDensity;
    float gravity;
};

[numthreads(1, 1, 1)]
void main(uint3 groupThreadID : SV_DispatchThreadID, int3 dispatchThreadID : SV_DispatchThreadID, int ID :SV_GroupIndex)
{
    float xPosParticle = groupThreadID.x / gravity;
    Particle tempParticle = particleOutput[groupThreadID.x];
    tempParticle.currentPosition = float3(xPosParticle, 0, 0);
    
    particleOutput[groupThreadID.x] = tempParticle;

}