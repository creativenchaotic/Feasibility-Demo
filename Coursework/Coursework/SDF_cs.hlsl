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

//Wave manipulation values used for Gerstner Waves
cbuffer TimeBuffer : register(b1)
{
    float time;
    float amplitude1;
    float frequency1;
    float speed1;
    
    float4 direction1;
    
    float amplitude2;
    float frequency2;
    float speed2;
    
    float steepnessFactor;
    
    float4 direction2;
    
    float amplitude3;
    float frequency3;
    float speed3;
    
    int isSampleWave;
    
    float4 direction3;
    
};


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

    float sphere1 = sdfSphere(position - ((float3(particleInitialPositions[0].xyz))), 1.0f); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(particleInitialPositions[1].xyz))), 1.0f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles > 2)
    {
        for (int i = 2; i < numParticles; i++)
        {
            float sphere = sdfSphere(position - ((float3(particleInitialPositions[i].xyz))), 1.0f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

//CALCULATE SDFs USING POSITIONS FROM SPH SIMULATION
float sdfCalculationsSPHSim(float3 position)
{
    float finalValue;

    float sphere1 = sdfSphere(position - ((float3(simulationOutputData[0].position))), 1.0f); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(simulationOutputData[1].position))), 1.0f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles > 2)
    {
        for (int i = 2; i < numParticles; i++)
        {
            float sphere = sdfSphere(position - ((float3(simulationOutputData[i].position))), 1.0f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}


//CALCULATE SDFs USING POSITIONS FROM GERSTNER WAVE CALCULATIONS
float sdfCalculationsSampleWave(float3 position)
{
    float finalValue;

    float sphere1 = sdfSphere(position - ((float3(particleData[0].xyz))), 1.0f); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(particleData[1].xyz))), 1.0f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles > 2)
    {
        for (int i = 2; i < numParticles; i++)
        {
            float sphere = sdfSphere(position - ((float3(particleData[i].xyz))), 1.0f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

void waterPlaneCalc()
{
    //PERFORM SUM OF SINES WITH GERSTNER WAVES ON THE PARTICLES
    for (int i = 0; i < numParticles; i++)
    {
        //RESETTING PARTICLE POSITIONS
        particleData[i].x = particleInitialPositions[i].x;
        particleData[i].z = particleInitialPositions[i].z;

    	//Changing the height of the water particles
        float wave1Pos = 2 * 0.585 * pow((((sin((dot(direction1.xz, float2(particleData[i].x, particleData[i].z))) * frequency1 + (time * (speed1 * frequency1)))) + 1) / 2), steepnessFactor);
        float wave2Pos = 2 * 0.245 * pow((((sin((dot(direction2.xz, float2(particleData[i].x, particleData[i].z))) * frequency2 + (time * (speed2 * frequency2)))) + 1) / 2), steepnessFactor);
        float wave3Pos = 2 * amplitude3 * pow((((sin((dot(direction3.xz, float2(particleData[i].x, particleData[i].z))) * frequency3 + (time * (speed3 * frequency3)))) + 1) / 2), steepnessFactor);


        particleData[i].y = wave1Pos + wave2Pos + wave3Pos;
    }
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

        if (isSampleWave == 1)//USE GERSTNER WAVE SAMPLE POSITIONS
        {
            waterPlaneCalc();
        	sdfCalc = sdfCalculationsSampleWave(position);
        }
        else//USE SPH SIMULATION
        {
        	sdfCalc = sdfCalculationsSPHSim(position);
        }

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
        if (isSampleWave == 1)//USE GERSTNER WAVES
        {
            waterPlaneCalc();
        }
        else//USE SPH SIMULATION
        {
            for (int i = 0; i < numParticles; i++)
            {
                particleData[i] = float4(simulationOutputData[i].position, 0);
            }
        }
    }

}