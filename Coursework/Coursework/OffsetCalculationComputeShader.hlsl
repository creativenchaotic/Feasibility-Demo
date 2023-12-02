static const int NumThreads = 128;

RWStructuredBuffer<int> particleOffsets : register(u0); //Data we pass to and from the compute shader

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= numEntries)
    {
        return;
    }
    uint i = dispatchThreadID.x;

    uint key = Entries[i].key;
    uint keyPrev = i == 0 ? 9999999 : Entries[i - 1].key;
    if (key != keyPrev)
    {
        Offsets[key] = i;
    }
}