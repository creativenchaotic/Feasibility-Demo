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
    uint3 spatialIndices; //x is the original index //y is the hash //z is the key
    float padding;
};

RWStructuredBuffer<int> particleOffsets : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<uint3> particleIndicesOutputFromBitonicMergesort : register(t0);

cbuffer cb_offsetCalculationsConstants : register(b0)
{
    int numParticles;
    float3 padding;
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= numParticles)
    {
        return;
    }
    uint i = dispatchThreadID.x;

    uint key = particleIndicesOutputFromBitonicMergesort[i].z;
    uint keyPrev = (i == 0) ? 9999999 : particleIndicesOutputFromBitonicMergesort[i - 1].z;
    if (key != keyPrev)
    {
        particleOffsets[key] = i;
    }

}