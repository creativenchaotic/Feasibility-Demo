static const int NumThreads = 128;

struct Particle
{
    int size;
    float3 startPosition;
    float3 currentPosition;
    float density;
    float3 predictedPosition;
    float nearDensity;
    float3 velocity;
    int spatialOffsets;
    float3 spatialIndices;
    float padding;
};

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
}