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
    // Sort the given entries by their keys (smallest to largest)
    // This is done using bitonic merge sort, and takes multiple iterations
    uint i = dispatchThreadID.x;

    uint hIndex = i & (groupWidth - 1);
    uint indexLeft = hIndex + (groupHeight + 1) * (i / groupWidth);
    uint rightStepSize = stepIndex == 0 ? groupHeight - 2 * hIndex : (groupHeight + 1) / 2;
    uint indexRight = indexLeft + rightStepSize;

	// Exit if out of bounds (for non-power of 2 input sizes)
    if (indexRight >= numEntries)
        return;

    uint valueLeft = Entries[indexLeft].key;
    uint valueRight = Entries[indexRight].key;

	// Swap entries if value is descending
    if (valueLeft > valueRight)
    {
        Entry temp = Entries[indexLeft];
        Entries[indexLeft] = Entries[indexRight];
        Entries[indexRight] = temp;
    }
}