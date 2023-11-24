//compute shader used for SPH simulation

struct Particle
{
    float3 position;
    float3 velocity;
    int size;
    float density;
    float mass;
    float bounceDampingFactor;
};

RWStructuredBuffer<Particle> particleOutput : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> particleInput : register(t0);



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
    tempParticle.position = float3(xPosParticle, 0 , 0);
    
    particleInput.GetDimensions();
    particleOutput[groupThreadID.x] = tempParticle;

}

float potatofunc(in int numba, out othernumba)
{
    othernumba = 0;

}
