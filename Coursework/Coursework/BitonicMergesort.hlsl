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

RWStructuredBuffer<int3> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> particleDataOutputFromSPHSimFirstPass : register(t0);

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
    particleData[dispatchThreadID.x] = particleDataOutputFromSPHSimFirstPass[dispatchThreadID.x].spatialIndices;
    
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

    uint valueLeft = particleData[indexLeft].z;
    uint valueRight = particleData[indexRight].z;

	// Swap entries if value is descending
    if (valueLeft > valueRight)
    {
        
        //Entry temp = Entries[indexLeft];
        //Entries[indexLeft] = Entries[indexRight];
        //Entries[indexRight] = temp;
        
        Entry temp;
        temp.originalIndex = particleData[indexLeft].x;
        temp.hash = particleData[indexLeft].y;
        temp.key = particleData[indexLeft].z;
        
        particleData[indexLeft] = particleData[indexRight];
        particleData[indexRight].x = temp.originalIndex;
        particleData[indexRight].y = temp.hash;
        particleData[indexRight].z = temp.key;
    }
   
    

}