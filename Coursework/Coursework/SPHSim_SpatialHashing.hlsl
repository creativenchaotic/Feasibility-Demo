//compute shader used for SPH simulation
static const int NumThreads = 64;

struct Particle
{
    int particleNum;
    float3 position;
    float3 predictedPosition;
    float3 velocity;
    float2 density;
    uint3 spatialIndices;
    uint spatialOffsets;
};

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> sphSimulationFirstPass : register(t0);

cbuffer cb_simConstants : register(b0)
{
    uint numParticles;
    float gravity;
    float deltaTime;
    float collisionsDamping;

    float smoothingRadius;
    float targetDensity;
    float pressureMultiplier;
    float nearPressureMultiplier;

    float viscosityStrength;
    float edgeForce;
    float edgeForceDst;
    float padding;

    float boundingBoxTop;
    float boundingBoxBottom;
    float boundingBoxLeftSide;
    float boundingBoxRightSide;

    float boundingBoxFront;
    float boundingBoxBack;
    float isFirstIteration;
    float padding2;
};

// Constants used for hashing
static const uint hashK1 = 15823;
static const uint hashK2 = 9737333;
static const uint hashK3 = 440817757;

// Convert floating point position into an integer cell coordinate
int3 GetCell3D(float3 position, float radius)
{
    return (int3) floor(position / radius);
}

// Hash cell coordinate to a single unsigned integer
uint HashCell3D(int3 cell)
{
    cell = (uint3) cell;
    return (cell.x * hashK1) + (cell.y * hashK2) + (cell.z * hashK3);
}

uint KeyFromHash(uint hash, uint tableSize)
{
    return hash % tableSize;
}


void UpdateSpatialHash(uint3 thread)
{
    if (thread.x >= numParticles)
        return;

	// Reset offsets
    particleData[thread.x].spatialOffsets = numParticles;
	// Update index buffer
    uint index = thread.x;
    int3 cell = GetCell3D(particleData[index].predictedPosition, smoothingRadius);
    uint hash = HashCell3D(cell);
    uint key = KeyFromHash(hash, numParticles);
    particleData[thread.x].spatialIndices = uint3(index, hash, key);
}

void SetValuesFromPreviousStage(uint3 thread)
{
    particleData[thread.x] = sphSimulationFirstPass[thread.x];
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    SetValuesFromPreviousStage(dispatchThreadID);
    UpdateSpatialHash(dispatchThreadID);
}