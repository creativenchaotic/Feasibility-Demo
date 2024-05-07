
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
    int isSampleWave;

    float boundingBoxTop;
    float boundingBoxBottom;
    float boundingBoxLeftSide;
    float boundingBoxRightSide;

    float boundingBoxFront;
    float boundingBoxBack;
    float2 padding2;

    float4x4 localToWorld;
    float4x4 worldToLocal;
};

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> sphDensityPassOutput : register(t0);


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

static const float PI = 3.1415926f;

float PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;
}

float NearPressureFromDensity(float nearDensity)
{
    return nearDensity * nearPressureMultiplier;
}

float DerivativeSpikyPow2(float dst, float radius)
{
    if (dst <= radius)
    {
        float scale = 15 / (pow(radius, 5) * PI);
        float v = radius - dst;
        return -v * scale;
    }
    return 0;
}

float DensityDerivative(float dst, float radius)
{
    return DerivativeSpikyPow2(dst, radius);
}

float DerivativeSpikyPow3(float dst, float radius)
{
    if (dst <= radius)
    {
        float scale = 45 / (pow(radius, 6) * PI);
        float v = radius - dst;
        return -v * v * scale;
    }
    return 0;
}

float NearDensityDerivative(float dst, float radius)
{
    return DerivativeSpikyPow3(dst, radius);
}

//SIMULATION FUNCTIONS---------------------------------------------------------
void CalculatePressureForce(uint3 thread)
{
    if (thread.x >= numParticles)
        return;

	// Calculate pressure
    float density = particleData[thread.x].density[0];
    float densityNear = particleData[thread.x].density[1];
    float pressure = PressureFromDensity(density);
    float nearPressure = NearPressureFromDensity(densityNear);
    float3 pressureForce = 0;
	
    float3 pos = particleData[thread.x].predictedPosition;
    int3 originCell = GetCell3D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

	// Neighbour search
    for (int i = 0; i < 27; i++)
    {
        uint hash = HashCell3D(originCell + offsets3D[i]);
        uint key = KeyFromHash(hash, numParticles);
        uint currIndex = particleData[key].spatialOffsets;

        while (currIndex < numParticles)
        {
            uint3 indexData = particleData[currIndex].spatialIndices;
            currIndex++;
			// Exit if no longer looking at correct bin
            if (indexData[2] != key)
                break;
			// Skip if hash does not match
            if (indexData[1] != hash)
                continue;

            uint neighbourIndex = indexData[0];
			// Skip if looking at self
            if (neighbourIndex == thread.x)
                continue;

            float3 neighbourPos = particleData[neighbourIndex].predictedPosition;
            float3 offsetToNeighbour = neighbourPos - pos;
            float sqrDstToNeighbour = dot(offsetToNeighbour, offsetToNeighbour);

			// Skip if not within radius
            if (sqrDstToNeighbour > sqrRadius)
                continue;

			// Calculate pressure force
            float densityNeighbour = particleData[neighbourIndex].density[0];
            float nearDensityNeighbour = particleData[neighbourIndex].density[1];
            float neighbourPressure = PressureFromDensity(densityNeighbour);
            float neighbourPressureNear = NearPressureFromDensity(nearDensityNeighbour);

            float sharedPressure = (pressure + neighbourPressure) / 2;
            float sharedNearPressure = (nearPressure + neighbourPressureNear) / 2;

            float dst = sqrt(sqrDstToNeighbour);
            float3 dir = dst > 0 ? offsetToNeighbour / dst : float3(0, 1, 0);

            pressureForce += dir * DensityDerivative(dst, smoothingRadius) * sharedPressure / densityNeighbour;
            pressureForce += dir * NearDensityDerivative(dst, smoothingRadius) * sharedNearPressure / nearDensityNeighbour;
        }
    }

    float3 acceleration = pressureForce / density;
    particleData[thread.x].velocity += acceleration * deltaTime;
}

void SetParticleValuesFromPreviousStage(uint3 thread)
{
    particleData[thread.x] = sphDensityPassOutput[thread.x];
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (isSampleWave == 1)
    {
	    
    }
    else
    {
        SetParticleValuesFromPreviousStage(dispatchThreadID);
        CalculatePressureForce(dispatchThreadID);
    }
}