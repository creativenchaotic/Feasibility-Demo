//compute shader used for SPH simulation

static const int NumThreads = 64;

//SPATIAL 3D HASH----------------------------------------------------------------
static const int3 offsets3D[27] =
{
    int3(-1, -1, -1),
	int3(-1, -1, 0),
	int3(-1, -1, 1),
	int3(-1, 0, -1),
	int3(-1, 0, 0),
	int3(-1, 0, 1),
	int3(-1, 1, -1),
	int3(-1, 1, 0),
	int3(-1, 1, 1),
	int3(0, -1, -1),
	int3(0, -1, 0),
	int3(0, -1, 1),
	int3(0, 0, -1),
	int3(0, 0, 0),
	int3(0, 0, 1),
	int3(0, 1, -1),
	int3(0, 1, 0),
	int3(0, 1, 1),
	int3(1, -1, -1),
	int3(1, -1, 0),
	int3(1, -1, 1),
	int3(1, 0, -1),
	int3(1, 0, 0),
	int3(1, 0, 1),
	int3(1, 1, -1),
	int3(1, 1, 0),
	int3(1, 1, 1)
};

// Constants used for hashing
static const uint hashK1 = 15823;
static const uint hashK2 = 9737333;
static const uint hashK3 = 440817757;


struct Particle
{
    int size;
    float3 startPosition;
    float3 currentPosition;
    float density;
    float3 predictedPosition;
    float nearDensity;
    float3 velocity;
    uint spatialOffsets;
    uint3 spatialIndices;
    float padding;
};

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> sphSimulationSecondPassOutput : register(t0);

cbuffer cb_simConstants : register(b0)
{
    int numParticles;
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


// Convert floating point position into an integer cell coordinate
int3 GetCell3D(float3 position, float radius)//CHANGED THIS FUNCTION BC THERES AN ERROR SOMEWHERE
{    
   // position = abs(position);
   // float x = floor(position.x / radius);
   // float y = floor(position.y / radius);
   // float z = floor(position.z / radius);
    
    //int ix = (int) x;
    //int iy = (int) y;
    //int iz = (int) z;
    
    //original
    return (int3) floor(position / radius);
    
    //return int3(x,y,z);
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

//SIMULATION FUNCTIONS---------------------------------------------------------
void ExternalForces(int3 thread)
{
    if (thread.x >= numParticles)
        return;

	// External forces (gravity)
    particleData[thread.x].velocity += float3(0, gravity, 0) * deltaTime;

	// Predict
    particleData[thread.x].predictedPosition = particleData[thread.x].currentPosition + particleData[thread.x].velocity * 1 / 120.0;
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
    int hash = HashCell3D(cell);
    int key = KeyFromHash(hash, numParticles);
    particleData[thread.x].spatialIndices = uint3(index, hash, key);
}

void setValuesFromPreviousIterationToCurrentIteration(int3 thread)
{
    particleData[thread.x] = (Particle)sphSimulationSecondPassOutput[thread.x];
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //TODO: Check if the buffer from the previous iteration of the simulation is empty
    if (isFirstIteration == 0)
    {
        setValuesFromPreviousIterationToCurrentIteration(dispatchThreadID);
    }
        
    ExternalForces(dispatchThreadID);
    UpdateSpatialHash(dispatchThreadID);
}

