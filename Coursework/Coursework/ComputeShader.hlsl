//compute shader used for SPH simulation

static const int NumThreads = 64;

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

StructuredBuffer<Particle> particleInput : register(t0);
RWStructuredBuffer<Particle> particleOutput : register(u0); //Data we pass to and from the compute shader

/*// Buffers
RWStructuredBuffer<float3> Positions : register(u1);
RWStructuredBuffer<float3> PredictedPositions : register(u2);
RWStructuredBuffer<float3> Velocities : register(u3);
RWStructuredBuffer<float2> Densities : register(u4); // Density, Near Density
RWStructuredBuffer<float3> SpatialIndices : register(u5); // used for spatial hashing
RWStructuredBuffer<int> SpatialOffsets : register(u6); // used for spatial hashing*/

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

    float2 boundingBoxTopAndBottom;
    float2 boudningBoxFrontAndBack;
    float2 boundingBoxSides;
    float2 padding2;
};

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

//----------------------------------------------------------------------------

static const float PI = 3.1415926;

 //HELPER FUNCTIONS-----------------------------------------------------------
float PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;

}

float NearPressureFromDensity(float nearDensity)
{
    return nearDensity * nearPressureMultiplier;
}

//TODO: Add resolve collisions function


//Integrate[(h-r)^2 r^2 Sin[?], {r, 0, h}, {?, 0, ?}, {?, 0, 2*?}]
float SpikyKernelPow2(float dst, float radius)
{
    if (dst < radius)
    {
        float scale = 15 / (2 * PI * pow(radius, 5));
        float v = radius - dst;
        return v * v * scale;
    }
    return 0;
}

float DensityKernel(float dst, float radius)
{
	//return SmoothingKernelPoly6(dst, radius);
    return SpikyKernelPow2(dst, radius);
}

float SpikyKernelPow3(float dst, float radius)
{
    if (dst < radius)
    {
        float scale = 15 / (PI * pow(radius, 6));
        float v = radius - dst;
        return v * v * v * scale;
    }
    return 0;
}

float NearDensityKernel(float dst, float radius)
{
    return SpikyKernelPow3(dst, radius);
}

//SIMULATION FUNCTIONS---------------------------------------------------------
void ExternalForces(int3 thread)
{
    if (thread.x >= numParticles)
        return;

	// External forces (gravity)
    particleInput[thread.x].velocity += float3(0, gravity, 0) * deltaTime;

	// Predict
    particleInput[thread.x].predictedPosition = particleInput[thread.x].currentPosition + particleInput[thread.x].velocity * 1 / 120.0;
}

void UpdateSpatialHash(int3 thread)
{
    if (thread.x >= numParticles)
        return;

	// Reset offsets
    particleInput[thread.x].spatialOffsets = numParticles;
	// Update index buffer
    uint index = thread.x;
    int3 cell = GetCell3D(particleInput[index].predictedPosition, smoothingRadius);
    uint hash = HashCell3D(cell);
    uint key = KeyFromHash(hash, numParticles);
    particleInput[thread.x].spatialIndices = uint3(index, hash, key);
}

void CalculateDensities(int3 thread)
{
    if (thread.x >= numParticles)
        return;

    float3 pos = particleInput[thread.x].predictedPosition;
    int3 originCell = GetCell3D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;
    float density = 0;
    float nearDensity = 0;

	// Neighbour search
    for (int i = 0; i < 27; i++)
    {
        uint hash = HashCell3D(originCell + offsets3D[i]);
        uint key = KeyFromHash(hash, numParticles);
        uint currIndex = particleInput[key].spatialOffsets;

        while (currIndex < numParticles)
        {
            uint3 indexData = particleInput[currIndex].spatialIndices;
            currIndex++;
			// Exit if no longer looking at correct bin
            if (indexData[2] != key)
                break;
			// Skip if hash does not match
            if (indexData[1] != hash)
                continue;

            uint neighbourIndex = indexData[0];
            float3 neighbourPos = particleInput[neighbourIndex].predictedPosition;
            float3 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

			// Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius)
                continue;

			// Calculate density and near density
            float dst = sqrt(sqrDstToNeighbour);
            density += DensityKernel(dst, smoothingRadius);
            nearDensity += NearDensityKernel(dst, smoothingRadius);
        }
    }
	
    particleInput[thread.x].density = density;
    particleInput[thread.x].nearDensity = nearDensity;
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    
   

}

