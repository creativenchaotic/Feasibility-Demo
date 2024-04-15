//compute shader used for SDF Rendering

static const int NumThreads = 64;

RWStructuredBuffer<float4> particleData : register(u0); //Data we pass to and from the compute shader. Currently the particle data, this should probably be an SRV instead since we are wanting to output the results of the SDF from this shader.

cbuffer cb_simConstants : register(b0){
    int numParticles;
    float blendAmount;
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

    float sphere1 = sdfSphere(position - float3(particleData[0].xyz), 1.f); //Sphere SDF
    float sphere2 = sdfSphere(position - float3(particleData[1].xyz), 1.f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles.x > 2)
    {
        for (int i = 2; i < numParticles.x; i++)
        {
            float sphere = sdfSphere(position - float3(particleData[i].xyz), 1.f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    /*
	//Setting up colours for SDF
    float3 finalColour = float3(0, 0, 0);
    float4 posColour = float4(0.0, 0.0, 1.0f, 1.0f);
    float4 negColour = float4(1.0, 0.0, 0.0, 1.0f);

    //Initialising variables used for raymarching
    float3 rayOrigin = float3(0, 0, -10.f);
    //float3 rayOrigin = cameraPos;
    float3 rayDirection = normalize(float3(input.tex, 1)); //Sets the direction of the ray to each point in the plane based on UVs
    float totalDistanceTravelled = 0.f; //Total distance travelled by ray from the camera's position


    //Raymarching (Sphere tracing)

    for (int i = 0; i < 100; i++)//The number of steps affects the quality of the results and the performance
    {
        float3 positionInRay = rayOrigin + rayDirection * totalDistanceTravelled; //Current position along the ray based on the distance from the rays origin

        float distanceToScene = sdfCalculations(positionInRay); //Current distance to the scene. Safe distance the point can travel to in any direction without overstepping an object

        totalDistanceTravelled += distanceToScene;

         //Colouring
        finalColour = float3(totalDistanceTravelled, totalDistanceTravelled, totalDistanceTravelled) / 100;

        if (distanceToScene < 0.001f || totalDistanceTravelled > 100.f)//If the distance to an SDF shape becomes smaller than 0.001 stop iterating //Stop iterating if the ray moves too far without hitting any objects
        {
            break;
        }

    }
*/
}