static const int NumThreads = 128;

struct Particle
{
    int particleNum;
    float3 position;
    float3 predictedPosition;
    float3 velocity;
    float2 density;
    uint3 spatialIndices; //x is the original index //y is the hash //z is the key
    uint spatialOffsets;
};

struct Entry
{
    //x is the original index
    //y is the hash
    //z is the key
    uint originalIndex;
    uint hash;
    uint key;
};

RWStructuredBuffer<Entry> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> particleDataOutputFromSPHSimFirstPass : register(t0);

cbuffer cb_bitonicMergesortConstants : register(b0)
{
    int numParticles;
    uint groupWidth;
    uint groupHeight;
    uint stepIndex;
}

void bitonicMergesort(uint3 dispatchThreadID)
{
    particleData[dispatchThreadID.x].originalIndex = particleDataOutputFromSPHSimFirstPass[dispatchThreadID.x].spatialIndices.x;
    particleData[dispatchThreadID.x].hash = particleDataOutputFromSPHSimFirstPass[dispatchThreadID.x].spatialIndices.y;
    particleData[dispatchThreadID.x].key = particleDataOutputFromSPHSimFirstPass[dispatchThreadID.x].spatialIndices.z;

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

    uint valueLeft = particleData[indexLeft].key;
    uint valueRight = particleData[indexRight].key;

		// Swap entries if value is descending
    if (valueLeft > valueRight)
    {
        Entry temp = particleData[indexLeft];
        particleData[indexLeft] = particleData[indexRight];
        particleData[indexRight] = temp;
    }

}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{   
    bitonicMergesort(dispatchThreadID);

}