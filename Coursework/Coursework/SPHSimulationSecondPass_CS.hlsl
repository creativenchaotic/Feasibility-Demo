
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
StructuredBuffer<Particle> sphSimulationFirstPassOutput : register(t0);
StructuredBuffer<uint3> bitonicMergesortParticleIndicesOutput : register(t1);
StructuredBuffer<int> particleOffsetCalculationsOutput : register(t2);

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

    float4x4 localToWorld;
    float4x4 worldToLocal;
};


//SPATIAL 3D HASH----------------------------------------------------------------
static const uint3 offsets3D[27] =
{
    uint3(-1, -1, -1),
	uint3(-1, -1, 0),
	uint3(-1, -1, 1),
	uint3(-1, 0, -1),
	uint3(-1, 0, 0),
	uint3(-1, 0, 1),
	uint3(-1, 1, -1),
	uint3(-1, 1, 0),
	uint3(-1, 1, 1),
	uint3(0, -1, -1),
	uint3(0, -1, 0),
	uint3(0, -1, 1),
	uint3(0, 0, -1),
	uint3(0, 0, 0),
	uint3(0, 0, 1),
	uint3(0, 1, -1),
	uint3(0, 1, 0),
	uint3(0, 1, 1),
	uint3(1, -1, -1),
	uint3(1, -1, 0),
	uint3(1, -1, 1),
	uint3(1, 0, -1),
	uint3(1, 0, 0),
	uint3(1, 0, 1),
	uint3(1, 1, -1),
	uint3(1, 1, 0),
	uint3(1, 1, 1)
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

 //HELPER FUNCTIONS-----------------------------------------------------------
void ResolveCollisions(int particleIndex)
{
    /*
    // Transform position/velocity to the local space of the bounding box (scale not included)
    float3 posLocal = mul(worldToLocal, float4(particleData[particleIndex].position, 1)).xyz;
    float3 velocityLocal = mul(worldToLocal, float4(particleData[particleIndex].velocity, 0)).xyz;

	// Calculate distance from box on each axis (negative values are inside box)
    const float3 halfSize = 0.5;
    const float3 edgeDst = halfSize - abs(posLocal);

	// Resolve collisions
    if (edgeDst.x <= 0)
    {
        posLocal.x = halfSize.x * sign(posLocal.x);
        velocityLocal.x *= -1 * collisionsDamping;
    }
    if (edgeDst.y <= 0)
    {
        posLocal.y = halfSize.y * sign(posLocal.y);
        velocityLocal.y *= -1 * collisionsDamping;
    }
    if (edgeDst.z <= 0)
    {
        posLocal.z = halfSize.z * sign(posLocal.z);
        velocityLocal.z *= -1 * collisionsDamping;
    }

	// Transform resolved position/velocity back to world space
    particleData[particleIndex].position = mul(localToWorld, float4(posLocal, 1)).xyz;
    particleData[particleIndex].velocity = mul(localToWorld, float4(velocityLocal, 0)).xyz;
    */

    
	// Resolve collisions
    //Resolving collisions in X-axis

    if (particleData[particleIndex].position.x <= boundingBoxLeftSide)
    {
        particleData[particleIndex].position.x = boundingBoxLeftSide;
        particleData[particleIndex].velocity.x *= -1 * collisionsDamping;
    }
    if (particleData[particleIndex].position.x >= boundingBoxRightSide)
    {
        particleData[particleIndex].position.x = boundingBoxRightSide;
        particleData[particleIndex].velocity.x *= -1 * collisionsDamping;
    }

    
    //Resolving collisions in Y-axis
    if (particleData[particleIndex].position.y <= boundingBoxBottom)
    {
        particleData[particleIndex].position.y = boundingBoxBottom;
        particleData[particleIndex].velocity.y *= -1 * collisionsDamping;
    }
    if (particleData[particleIndex].position.y >= boundingBoxTop)
    {
        particleData[particleIndex].position.y = boundingBoxTop;
        particleData[particleIndex].velocity.y *= -1 * collisionsDamping;
    }

    
    //Resolving collisions in Z-axis
    if (particleData[particleIndex].position.z <= boundingBoxFront)
    {
        particleData[particleIndex].position.z = boundingBoxFront;
        particleData[particleIndex].velocity.z *= -1 * collisionsDamping;
    }
    if (particleData[particleIndex].position.z >= boundingBoxBack)
    {
        particleData[particleIndex].position.z = boundingBoxBack;
        particleData[particleIndex].velocity.z *= -1 * collisionsDamping;
    }

}

float PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;
}

float NearPressureFromDensity(float nearDensity)
{
    return nearDensity * nearPressureMultiplier;
}


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

float SmoothingKernelPoly6(float dst, float radius)
{
    if (dst < radius)
    {
        float scale = 315 / (64 * PI * pow(abs(radius), 9));
        float v = radius * radius - dst * dst;
        return v * v * v * scale;
    }
    return 0;
}


//SIMULATION FUNCTIONS---------------------------------------------------------
void CalculatePressureForce(uint3 thread)
{
    if (thread.x >= numParticles)
        return;
    
	// Calculate pressure
    float density = particleData[thread.x].density.x;
    float densityNear = particleData[thread.x].density.y;
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
            float densityNeighbour = particleData[neighbourIndex].density.x;
            float nearDensityNeighbour = particleData[neighbourIndex].density.y;
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


void CalculateDensities(uint3 thread)
{
    if (thread.x >= numParticles)
        return;

    float3 pos = particleData[thread.x].predictedPosition;
    int3 originCell = GetCell3D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;
    float density = 0;
    float nearDensity = 0;

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
            float3 neighbourPos = particleData[neighbourIndex].predictedPosition;
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
	
    particleData[thread.x].density = float2(density, nearDensity);
}


void CalculateViscosity(uint3 thread)
{
    if (thread.x >= numParticles)
        return;
		
    float3 pos = particleData[thread.x].predictedPosition;
    int3 originCell = GetCell3D(pos, smoothingRadius);
    float sqrRadius = smoothingRadius * smoothingRadius;

    float3 viscosityForce = 0;
    float3 velocity = particleData[thread.x].velocity;

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

			// Calculate viscosity
            float dst = sqrt(sqrDstToNeighbour);
            float3 neighbourVelocity = particleData[neighbourIndex].velocity;
            viscosityForce += (neighbourVelocity - velocity) * SmoothingKernelPoly6(dst, smoothingRadius);
        }

        particleData[thread.x].velocity += viscosityForce * viscosityStrength * deltaTime;
    }
}

void UpdatePositions(uint3 thread)
{
    if (thread.x >= numParticles)
        return;
    
    particleData[thread.x].position += particleData[thread.x].velocity * deltaTime;

    ResolveCollisions(thread.x);
}

void SetParticleDataOffsetsAndIndices(uint3 thread)
{
    particleData[thread.x] = sphSimulationFirstPassOutput[thread.x];
    particleData[thread.x].spatialIndices = bitonicMergesortParticleIndicesOutput[thread.x];
    particleData[thread.x].spatialOffsets = particleOffsetCalculationsOutput[thread.x];
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    SetParticleDataOffsetsAndIndices(dispatchThreadID);
   
    CalculateDensities(dispatchThreadID);
    CalculatePressureForce(dispatchThreadID);
    CalculateViscosity(dispatchThreadID);
    UpdatePositions(dispatchThreadID);
}