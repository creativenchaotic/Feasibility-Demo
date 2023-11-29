//compute shader used for SPH simulation

static const int NumThreads = 64;

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

/*// Buffers
RWStructuredBuffer<float3> Positions : register(u1);
RWStructuredBuffer<float3> PredictedPositions : register(u2);
RWStructuredBuffer<float3> Velocities : register(u3);
RWStructuredBuffer<float2> Densities : register(u4); // Density, Near Density
RWStructuredBuffer<float3> SpatialIndices : register(u5); // used for spatial hashing
RWStructuredBuffer<int> SpatialOffsets : register(u6); // used for spatial hashing*/

cbuffer cb_simConstants : register(b0)
{
    int numParticles;
    float gravity;
    float deltaTime;
    float collisionsDamping;

    float smoothingRadius;
    float targetDensity;
    float pressureMultiplier;
    float nearPressureMultiplier;

    float viscosityStrength;
    float edgeForce;
    float edgeForceDst;
    float padding;

    float2 boundingBoxTopAndBottom;
    float2 boudningBoxFrontAndBack;
    float2 boundingBoxSides;
    float2 padding2;
};
 
float PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;

}

float NearPressureFromDensity(float nearDensity)
{
    return nearDensity * nearPressureMultiplier;
}

//TODO: Add resolve collisions function



[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    

}