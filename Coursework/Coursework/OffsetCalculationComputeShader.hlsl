static const int NumThreads = 128;

RWStructuredBuffer<int> particleOffsets : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<uint3> particleIndicesOutputFromBitonicMergesort : register(t0);

cbuffer cb_offsetCalculationsConstants : register(b0)
{
    int numParticles;
    float3 padding;
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
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