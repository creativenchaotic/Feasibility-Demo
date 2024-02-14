

cbuffer CameraBuffer : register(b0){
    float4 cameraPos;
}


struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float sdfSphere(float3 position, float radius)
{
    //We return the length of the vector that describes the distance between the camera and the surface of the sphere
    //This would be the length of the camera vector - the position of the centre of the sphere - the radius of the sphere

    return length(position) - radius;//Got this from Inigo Quilez
}


float smoothUnion(float shapeA, float shapeB, float blendingAmount)
{
    float h = max(blendingAmount - abs(shapeA - shapeB), 0.0f) / blendingAmount;
    return min(shapeA, shapeB) - h * h * h * blendingAmount * (1.0f / 6.0f);
}

float sdfCalculations(float3 position)
{
    float3 spherePosition1 = float3(5,5,5);//Position of the sphere in SDF Object World
    float sphere1 = sdfSphere(position - spherePosition1, 5.f); //Sphere SDF

    float3 spherePosition2 = float3(0, 5, 5); //Position of the sphere in SDF Object World
    float sphere2 = sdfSphere(position - spherePosition2, 5.f); //Sphere SDF


    return smoothUnion(sphere1, sphere2, 2.0f);
}


float4 main(InputType input) : SV_TARGET
{
    //Setting up colours for SDF
    float3 finalColour = float3(0,0,0);
    float4 posColour = float4(0.0, 0.0, 1.0f, 1.0f);
    float4 negColour = float4(1.0, 0.0, 0.0, 1.0f);

    //Initialising variables used for raymarching
    float3 rayOrigin = float3(0,0,-3.f);
    float3 rayDirection = normalize(float3(input.tex, 1)); //Sets the direction of the ray to each point in the plane based on UVs
    float totalDistanceTravelled = 0.f; //Total distance travelled by ray from the camera's position


    //Raymarching (Sphere tracing)

    for (int i = 0; i < 80; i++)//The number of steps affects the quality of the results and the performance
    {
        float3 positionInRay = rayOrigin + rayDirection * totalDistanceTravelled; //Current position along the ray based on the distance from the rays origin

        float distanceToScene = sdfCalculations(positionInRay); //Current distance to the scene. Safe distance the point can travel to in any direction without overstepping an object

        totalDistanceTravelled += distanceToScene;

         //Colouring
        finalColour = float3(totalDistanceTravelled, totalDistanceTravelled, totalDistanceTravelled) /80;

        if (distanceToScene < 0.001f || totalDistanceTravelled > 100.f)//If the distance to an SDF shape becomes smaller than 0.001 stop iterating //Stop iterating if the ray moves too far without hitting any objects
        {
            break;
        }
           
        
        
    }

   


    //float distanceToSphere = sdfSphere(cameraPos, input.position.xyz, 1);
    
    //finalColour = distanceToSphere > 0.0f ? posColour : negColour;

	//finalColour = finalColour * exp(distanceToSphere);

	//return finalColour;

    return float4(finalColour,1.0f);
}