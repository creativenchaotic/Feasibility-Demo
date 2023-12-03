static const int NumThreads = 128;

struct Entry
{
    float originalIndex;
    float hash;
    float key;
};

RWStructuredBuffer<float3> particleIndices : register(u0); //Data we pass to and from the compute shader
//x is the original index
//y is the hash
//z is the key

cbuffer cb_bitonicMergesortConstants : register(b0)
{
    int numParticles;
    int groupWidth;
    int groupHeight;
    int stepIndex;
}

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
    if (indexRight >= numParticles)
        return;

    uint valueLeft = particleIndices[indexLeft].z;
    uint valueRight = particleIndices[indexRight].z;

	// Swap entries if value is descending
    if (valueLeft > valueRight)
    {
        Entry temp;
        temp.originalIndex= particleIndices[indexLeft].x;
        temp.hash = particleIndices[indexLeft].y;
        temp.key = particleIndices[indexLeft].z;
        
        particleIndices[indexLeft] = particleIndices[indexRight];
        particleIndices[indexRight].x = temp.originalIndex;
        particleIndices[indexRight].y = temp.hash;
        particleIndices[indexRight].z = temp.key;
    }
}