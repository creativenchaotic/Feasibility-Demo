//compute shader used for SPH simulation

struct Particle
{
    float3 position;
    float3 velocity;
};

Texture2D gInput : register(t0);
RWStructuredBuffer<Particle> particleOutput : register(u0);//Data we pass to and from the compute shader

//Main idea would be to have a buffer to send in all the particle data and then another buffer to output all the data post simulation
//I dont really understand compute shaders ngl

float gravity = 9.8f;

[numthreads(1, 1, 1)]
void main(uint3 groupThreadID : SV_DispatchThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    float xPosParticle = groupThreadID.x / gravity;
    Particle tempParticle = particleOutput[groupThreadID.x];
    tempParticle.position = float3(xPosParticle, 0 , 0);
    
    particleOutput[groupThreadID.x] = tempParticle;

}