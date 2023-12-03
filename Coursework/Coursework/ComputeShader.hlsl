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
static const int hashK1 = 15823;
static const int hashK2 = 9737333;
static const int hashK3 = 440817757;


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
    int3 spatialIndices;
    float padding;
};

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader


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
    float2 padding2;
};


// Convert floating point position into an integer cell coordinate
int3 GetCell3D(float3 position, float radius)//CHANGED THIS FUNCTION BC THERES AN ERROR SOMEWHERE
{
    /*int x = floor(position.x/radius);
    int y = floor(position.y/ radius);
    int z = floor(position.z/radius);
    
    return (int3(x,y,z));*/
    
    return (int3) floor(position / radius);
}

// Hash cell coordinate to a single unsigned integer
uint HashCell3D(int3 cell)
{
    //return ((abs(cell.x) * hashK1) + (abs(cell.y) * hashK2) + (abs(cell.z) * hashK3));
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


void UpdateSpatialHash(int3 thread)
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
    particleData[thread.x].spatialIndices.x = index;
    particleData[thread.x].spatialIndices.y = hash;
    particleData[thread.x].spatialIndices.z = key;
}


[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    ExternalForces(dispatchThreadID);
    UpdateSpatialHash(dispatchThreadID);
}

