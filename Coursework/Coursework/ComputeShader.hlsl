//compute shader used for SPH simulation

struct Particle
{
    int size;
    float3 padding;
    float3 startPosition;
    float density;
    float3 currentPosition;
    float mass;
    float3 velocity;
    float bounceDampingFactor;
};

StructuredBuffer<Particle> particleInput : register(t0);
RWStructuredBuffer<Particle> particleOutput : register(u0); //Data we pass to and from the compute shader

cbuffer cb_simConstants : register(b0)
{
    float bounceDampingFactor;
    float gravity;
    float numParticles;
    float restDensity;
    float deltaTime;
    float2 boundingBoxTopAndBottom;
    float2 boundingBoxFrontAndBack;
    float2 boundingBoxSides;
    float smoothingRadius;
    float mass;
};


void resolveSimmulationBounds(uint3 groupThread)
{
    //Check bounds in X
    if (particleInput[groupThread.y].currentPosition.x < boundingBoxSides.x)
    {
        particleInput[groupThread.y].currentPosition.x = boundingBoxSides.x;
        particleInput[groupThread.y].velocity.x *= -1 * bounceDampingFactor;

    }
    else if (particleInput[groupThread.y].currentPosition.x > boundingBoxSides.y)
    {
        particleInput[groupThread.y].currentPosition.x = boundingBoxSides.y;
        particleInput[groupThread.y].velocity.x *= -1 * bounceDampingFactor;
    }

    //Check bounds in Y
    if (particleInput[groupThread.y].currentPosition.y < boundingBoxTopAndBottom.x)
    {
        particleInput[groupThread.y].currentPosition.y = boundingBoxTopAndBottom.x;
        particleInput[groupThread.y].velocity.y *= -1 * bounceDampingFactor;
    }
    else if (particleInput[groupThread.y].currentPosition.y > boundingBoxTopAndBottom.y)
    {
        particleInput[groupThread.y].currentPosition.y = boundingBoxTopAndBottom.y;
        particleInput[groupThread.y].velocity.y *= -1 * bounceDampingFactor;
    }
    
    //Check bounds in Z
    if (particleInput[groupThread.y].currentPosition.z < boundingBoxFrontAndBack.x)
    {
        particleInput[groupThread.y].currentPosition.z = boundingBoxFrontAndBack.x;
        particleInput[groupThread.y].velocity.z *= -1 * bounceDampingFactor;
    }
    else if (particleInput[groupThread.y].currentPosition.z > boundingBoxFrontAndBack.y)
    {
        particleInput[groupThread.y].currentPosition.z = boundingBoxFrontAndBack.y;
        particleInput[groupThread.y].velocity.z *= -1 * bounceDampingFactor;
    }
}

//Used to smooth the particles down into a continuous field
float smoothingKernel(float radius, float distance)//Sebastian Lague's smoothing kernel. Can be substituted with another one
{
    float volume = 3.14f * pow(radius, 8) / 4;
    float value = max(0, radius*radius - distance*distance);
    return value * value * value / volume;//Keeping the volume the same even if the smoothing radius changes so that the density stays consistent
}

float calculateDensity(float3 samplePoint)//Calculating the density at a point in the smoothed particle field
{
    float density = 0;
    
    for (int i = 0; i < numParticles; i++)//For each particle
    {
        //Calculate the magnitude of the distance from the particle to the sample point
        float3 distanceFromSamplePoint = (particleInput[i].currentPosition - samplePoint);
        float distanceFromSamplePointMagnitude = sqrt(distanceFromSamplePoint.x * distanceFromSamplePoint.x + distanceFromSamplePoint.y * distanceFromSamplePoint.y + distanceFromSamplePoint.z * distanceFromSamplePoint.z);
        
        //Determine how much influence the particle has at that location
        float influence = smoothingKernel(smoothingRadius, distanceFromSamplePointMagnitude);
        
        density += mass * influence;
        
        return density;
    }

}


groupshared Particle cache[numParticles];
 
//Runs through all the particles??
[numthreads(1, numParticles, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{

    particleInput[groupThreadID.y].velocity.xy += float2(0.0f, 1.0f) * gravity * deltaTime;
    particleInput[groupThreadID.y].currentPosition.xy += particleInput[groupThreadID.y].velocity.xy * deltaTime;
    particleOutput[dispatchThreadID.y] = particleInput[groupThreadID.y];
    
    resolveSimmulationBounds(groupThreadID);

}
