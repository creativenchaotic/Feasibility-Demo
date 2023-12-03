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
    int3 spatialIndices; //x is the original index //y is the hash //z is the key
    float padding;
};

struct Entry
{
    //x is the original index
    //y is the hash
    //z is the key
    float originalIndex;
    float hash;
    float key;
};

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> particleDataOutputFromSPHSimFirstPass : register(t0);

cbuffer cb_bitonicMergesortConstants : register(b0)
{
    int numParticles;
    int groupWidth;
    int groupHeight;
    int stepIndex;
}

void settingParticleDataForNextComputeShader(int3 thread)
{
    particleData[thread.x].currentPosition = particleDataOutputFromSPHSimFirstPass[thread.x].currentPosition;
    particleData[thread.x].density = particleDataOutputFromSPHSimFirstPass[thread.x].density;
    particleData[thread.x].nearDensity = particleDataOutputFromSPHSimFirstPass[thread.x].nearDensity;
    particleData[thread.x].padding = particleDataOutputFromSPHSimFirstPass[thread.x].padding;
    particleData[thread.x].predictedPosition = particleDataOutputFromSPHSimFirstPass[thread.x].predictedPosition;
    particleData[thread.x].size = particleDataOutputFromSPHSimFirstPass[thread.x].size;
    particleData[thread.x].spatialOffsets = particleDataOutputFromSPHSimFirstPass[thread.x].spatialOffsets;
    particleData[thread.x].startPosition = particleDataOutputFromSPHSimFirstPass[thread.x].startPosition;
    particleData[thread.x].velocity = particleDataOutputFromSPHSimFirstPass[thread.x].velocity;
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

    uint valueLeft = particleDataOutputFromSPHSimFirstPass[indexLeft].spatialIndices.z;
    uint valueRight = particleDataOutputFromSPHSimFirstPass[indexRight].spatialIndices.z;

	// Swap entries if value is descending
    if (valueLeft > valueRight)
    {
        Entry temp;
        temp.originalIndex = particleDataOutputFromSPHSimFirstPass[indexLeft].spatialIndices.x;
        temp.hash = particleDataOutputFromSPHSimFirstPass[indexLeft].spatialIndices.y;
        temp.key = particleDataOutputFromSPHSimFirstPass[indexLeft].spatialIndices.z;
        
        particleData[indexLeft].spatialIndices = particleData[indexRight].spatialIndices;
        particleData[indexRight].spatialIndices.x = temp.originalIndex;
        particleData[indexRight].spatialIndices.y = temp.hash;
        particleData[indexRight].spatialIndices.z = temp.key;
    }
    
    settingParticleDataForNextComputeShader(dispatchThreadID);

}