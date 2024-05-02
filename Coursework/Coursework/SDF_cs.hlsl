//compute shader used to create a 3D texture out of the SDF calculations

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

RWStructuredBuffer<float4> particleData : register(u0);
RWTexture3D<float> SDFImage : register(u1);
StructuredBuffer<Particle> simulationOutputData : register(t0);
StructuredBuffer<float4> particleInitialPositions : register(t1);


cbuffer cb_simConstants : register(b0){
    int numParticles;
    float blendAmount;
    int stride;
    float offset;
    float4 simType;
}


float sdfSphere(float3 position, float radius)
{
    //We return the length of the vector that describes the distance between the camera and the surface of the sphere
    //This would be the length of the camera vector - the position of the centre of the sphere - the radius of the sphere

    return length(position) - radius; //Got this from Inigo Quilez
}


float smoothUnion(float shapeA, float shapeB)
{
    float h = max(blendAmount - abs(shapeA - shapeB), 0.0f) / blendAmount;
    return min(shapeA, shapeB) - h * h * h * blendAmount * (1.0f / 6.0f);
}


float sdfCalculations(float3 position)
{
        
    float finalValue;

    float sphere1 = sdfSphere(position - ((float3(particleData[0].xyz))), 1.0f); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(particleData[1].xyz))), 1.0f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles.x > 2)
    {
        for (int i = 2; i < numParticles.x; i++)
        {
            float sphere = sdfSphere(position - ((float3(particleData[i].xyz))), 1.0f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}


float sdfCalculationsSPHSim(float3 position)
{
        
    float finalValue;

    float sphere1 = sdfSphere(position - ((float3(simulationOutputData[0].position))), 1.0f); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(simulationOutputData[1].position))), 1.0f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles.x > 2)
    {
        for (int i = 2; i < numParticles.x; i++)
        {
            float sphere = sdfSphere(position - ((float3(simulationOutputData[i].position))), 1.0f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID)
{
    if (simType.x == 0)
    {
        uint3 resolution;
        SDFImage.GetDimensions(resolution.x, resolution.y, resolution.z);

        float3 worldMin = float3(-offset, -offset, -offset);
        float3 worldMax = float3(offset, offset, offset);

        float3 position = lerp(worldMin, worldMax, DTid / (float3(resolution) - 1.0f)) * 0.5;

        float sdfCalc = sdfCalculations(position);
        SDFImage[DTid.xyz] = sdfCalc; // Assign the calculated SDF value to the corresponding texel
    }
    else if(simType.x == 1)
    {
        uint3 resolution;
        SDFImage.GetDimensions(resolution.x, resolution.y, resolution.z);

        float3 worldMin = float3(-offset, -offset, -offset);
        float3 worldMax = float3(offset, offset, offset);

        float3 position = lerp(worldMin, worldMax, DTid / (float3(resolution) - 1.0f)) * 0.5;

        float sdfCalc = sdfCalculationsSPHSim(position);
        SDFImage[DTid.xyz] = sdfCalc; // Assign the calculated SDF value to the corresponding texel
    }
    else if(simType.x == 2)
    {
        for (int i = 0; i < numParticles; i++)
        {
            particleData[i] = particleInitialPositions[i];
        }
    }
    else if(simType.x == 3)
    {
        for (int i = 0; i < numParticles; i++)
        {
            particleData[i] = float4(simulationOutputData[i].position, 0);
        }
    }

}