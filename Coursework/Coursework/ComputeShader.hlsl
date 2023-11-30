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
RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader

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

cbuffer cb_matrices : register(b1)
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
}


//SPATIAL 3D HASH----------------------------------------------------------------
int3 offsets3D[27] =
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
uint hashK1 = 15823;
uint hashK2 = 9737333;
uint hashK3 = 440817757;

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

float PI = 3.1415926;

 //HELPER FUNCTIONS-----------------------------------------------------------
float PressureFromDensity(float density)
{
    return (density - targetDensity) * pressureMultiplier;

}

float NearPressureFromDensity(float nearDensity)
{
    return nearDensity * nearPressureMultiplier;
}

void ResolveCollisions(int particleIndex)
{
	// Transform position/velocity to the local space of the bounding box (scale not included)
    float3 posLocal = mul(worldToLocal, float4(particleData[particleIndex].currentPosition, 1)).xyz;
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
    particleData[particleIndex].currentPosition = mul(localToWorld, float4(posLocal, 1)).xyz;
    particleData[particleIndex].velocity = mul(localToWorld, float4(velocityLocal, 0)).xyz;

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
    uint hash = HashCell3D(cell);
    uint key = KeyFromHash(hash, numParticles);
    particleData[thread.x].spatialIndices = uint3(index, hash, key);
}


void CalculateDensities(int3 thread)
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
	
    particleData[thread.x].density = density;
    particleData[thread.x].nearDensity = nearDensity;
}

void CalculatePressureForce(int3 thread)
{
    if (thread.x >= numParticles)
        return;

	// Calculate pressure
    float density = particleData[thread.x].density;
    float densityNear = particleData[thread.x].nearDensity;
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
            float densityNeighbour = particleData[neighbourIndex].density;
            float nearDensityNeighbour = particleData[neighbourIndex].nearDensity;
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

void CalculateViscosity(int3 thread)
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

void UpdatePositions(int3 thread)
{
    if (thread.x >= numParticles)
        return;

    particleData[thread.x].currentPosition += particleData[thread.x].velocity * deltaTime;
    ResolveCollisions(thread.x);
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    
   

}

