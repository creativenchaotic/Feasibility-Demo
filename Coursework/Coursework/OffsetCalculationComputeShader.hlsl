static const int NumThreads = 128;

struct Entry
{
    //x is the original index
    //y is the hash
    //z is the key
    uint originalIndex;
    uint hash;
    uint key;
};

RWStructuredBuffer<uint> particleOffsets : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Entry> particleIndicesOutputFromBitonicMergesort : register(t0);

cbuffer cb_offsetCalculationsConstants : register(b0)
{
    uint numParticles;
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

    uint key = particleIndicesOutputFromBitonicMergesort[i].key;
    uint keyPrev = i == 0 ? 9999999 : particleIndicesOutputFromBitonicMergesort[i - 1].key;
    if (key != keyPrev)
    {
        particleOffsets[key] = i;
    }

}