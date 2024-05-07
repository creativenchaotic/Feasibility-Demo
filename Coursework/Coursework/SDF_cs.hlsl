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
    float particleSize;
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

//CALCULATE SDFs BASED ON STATIC PARTICLE DATA
float sdfCalculations(float3 position)
{
        
    float finalValue;

    float sphere1 = sdfSphere(position - ((float3(particleInitialPositions[0].xyz))), particleSize); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(particleInitialPositions[1].xyz))), particleSize); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles > 2)
    {
        for (int i = 2; i < numParticles; i++)
        {
            float sphere = sdfSphere(position - ((float3(particleInitialPositions[i].xyz))), particleSize); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

//CALCULATE SDFs USING POSITIONS FROM SPH SIMULATION
float sdfCalculationsSPHSim(float3 position)
{
    float finalValue;

    float sphere1 = sdfSphere(position - ((float3(simulationOutputData[0].position))), particleSize); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(simulationOutputData[1].position))), particleSize); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles > 2)
    {
        for (int i = 2; i < numParticles; i++)
        {
            float sphere = sdfSphere(position - ((float3(simulationOutputData[i].position))), particleSize); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}


[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID)
{
    ///THE SCENE CAN BE RENDERED EITHER USING 3D TEXTURES OR NORMAL SDFs, THIS SHADER IS USED TO SET UP THE 3D TEXTURE/THE UAVs TO SEND DATA TO THE PIXEL SHADER
    ///TO RENDER
    ///
    ///THE BRANCHING IF STATEMENTS MAKE THIS LESS EFFICIENT, THIS SHOULD PROBABLY BE SET UP AS MULTIPLE COMPUTE SHADERS INSTEAD THAT GET CALLED ON THE CPU BASED
    ///ON DIFFERENT OPTIONS
    
    //3D TEXTURE USING STATIC PARTICLES IN THE INITIAL POSITIONS OF THE SPH SIMULATION--------------------
    if (simType.x == 0)
    {
        uint3 resolution;
        SDFImage.GetDimensions(resolution.x, resolution.y, resolution.z);

        float3 worldMin = float3(-offset, -offset, -offset);
        float3 worldMax = float3(offset, offset, offset);

        float3 position = lerp(worldMin, worldMax, DTid / (float3(resolution) - 1.0f)) * 0.5;

        float sdfCalc = sdfCalculations(position);
        SDFImage[DTid] = sdfCalc; // Assign the calculated SDF value to the corresponding texel
    }

    //3D TEXTURE USING SIMULATED PARTICLES-----------------------------------------------------------------
    else if(simType.x == 1)
    {
        float sdfCalc;

        uint3 resolution;
        SDFImage.GetDimensions(resolution.x, resolution.y, resolution.z);

        float3 worldMin = float3(-offset, -offset, -offset);
        float3 worldMax = float3(offset, offset, offset);

        float3 position = lerp(worldMin, worldMax, DTid / (float3(resolution) - 1.0f)) * 0.5;

 
        sdfCalc = sdfCalculationsSPHSim(position);
        

        SDFImage[DTid] = sdfCalc; // Assign the calculated SDF value to the corresponding texel
    }

    //USE BASIC SDFs WITH SPHERE TRACING IN PIXEL SHADER USING STATIC PARTICLES-------------------------------
    else if(simType.x == 2)
    {
        for (int i = 0; i < numParticles; i++)
        {
            particleData[i] = particleInitialPositions[i];
        }
    }

    //USE BASIC SDFs WITH SPHERE TRACING IN THE PIXEL SHADER USING SIMULATED PARTICLES------------------------
    else if(simType.x == 3)
    {

        for (int i = 0; i < numParticles; i++)
        {
            particleData[i] = float4(simulationOutputData[i].position, 0);
        }
        
    }

}