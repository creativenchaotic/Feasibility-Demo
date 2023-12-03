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

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> particleDataOutputFromBitonicMergesort : register(t0);


cbuffer cb_offsetCalculationsConstants : register(b0)
{
    int numParticles;
    float3 padding;
}

void settingParticleDataForNextComputeShader(int3 thread)
{
    particleData[thread.x].currentPosition = particleDataOutputFromBitonicMergesort[thread.x].currentPosition;
    particleData[thread.x].density = particleDataOutputFromBitonicMergesort[thread.x].density;
    particleData[thread.x].nearDensity = particleDataOutputFromBitonicMergesort[thread.x].nearDensity;
    particleData[thread.x].padding = particleDataOutputFromBitonicMergesort[thread.x].padding;
    particleData[thread.x].predictedPosition = particleDataOutputFromBitonicMergesort[thread.x].predictedPosition;
    particleData[thread.x].size = particleDataOutputFromBitonicMergesort[thread.x].size;
    particleData[thread.x].spatialIndices = particleDataOutputFromBitonicMergesort[thread.x].spatialIndices;
    particleData[thread.x].startPosition = particleDataOutputFromBitonicMergesort[thread.x].startPosition;
    particleData[thread.x].velocity = particleDataOutputFromBitonicMergesort[thread.x].velocity;
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= numParticles)
    {
        return;
    }
    uint i = dispatchThreadID.x;

    uint key = particleDataOutputFromBitonicMergesort[i].spatialIndices.z;
    uint keyPrev = i == 0 ? 9999999 : particleDataOutputFromBitonicMergesort[i - 1].spatialIndices.z;
    if (key != keyPrev)
    {
        particleData[key].spatialOffsets = i;
    }
    
    settingParticleDataForNextComputeShader(dispatchThreadID);

}