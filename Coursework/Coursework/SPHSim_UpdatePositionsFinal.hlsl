
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

RWStructuredBuffer<Particle> particleData : register(u0); //Data we pass to and from the compute shader
StructuredBuffer<Particle> sphViscosityPassOutput : register(t0);

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


void UpdatePositions(uint3 thread)
{
    if (thread.x >= numParticles)
        return;

    particleData[thread.x].position += particleData[thread.x].velocity * deltaTime;
    ResolveCollisions(thread.x);
}


void SetParticleValuesFromPreviousStage(uint3 thread)
{
    particleData[thread.x] = sphViscosityPassOutput[thread.x];
}

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    SetParticleValuesFromPreviousStage(dispatchThreadID);
    UpdatePositions(dispatchThreadID);
}