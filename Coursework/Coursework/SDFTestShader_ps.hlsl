

cbuffer CameraBuffer : register(b0){
    float4 cameraPos;
    float4 timer;
}
cbuffer SDFBuffer : register (b1){
    float4 blendAmount;
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


float smoothUnion(float shapeA, float shapeB)
{
    float h = max(blendAmount - abs(shapeA - shapeB), 0.0f) / blendAmount;
    return min(shapeA, shapeB) - h * h * h * blendAmount * (1.0f / 6.0f);
}

float sdfCalculations(float3 position)
{
    float sine = sin(timer) * 5.0f;
    float cosine = cos(timer) * 2.0f;

    float sine2 = sin(timer) * 10.0f;
    float cosine2 = cos(timer) * 20.0f;

    
    //1st BIT TO SHOW------------------------------------------------------------------------------
    float3 spherePosition1 = float3(0, 5, 40); //Position of the sphere in SDF Object World
    float sphere1 = sdfSphere(position - spherePosition1, 5.f); //Sphere SDF

    float3 spherePosition2 = float3(10, 10, 40); //Position of the sphere in SDF Object World
    float sphere2 = sdfSphere(position - spherePosition2, 5.f); //Sphere SDF


    return smoothUnion(sphere1, sphere2);
    //---------------------------------------------------------------------------------------------

    /*
    //2nd BIT TO SHOW------------------------------------------------------------------------------
    float3 spherePosition1 = float3(30, 45, 70); //Position of the sphere in SDF Object World
    float sphere1 = sdfSphere(position - spherePosition1, 5.f); //Sphere SDF

    float3 spherePosition2 = float3(sine2 + 20.f, 10, 70); //Position of the sphere in SDF Object World
    float sphere2 = sdfSphere(position - spherePosition2, 2.f); //Sphere SDF

    float3 spherePosition3 = float3(5, cosine2 + 10, 70); //Position of the sphere in SDF Object World
    float sphere3 = sdfSphere(position - spherePosition3, 10.f); //Sphere SDF

    float3 spherePosition4 = float3(15, 20, 70); //Position of the sphere in SDF Object World
    float sphere4 = sdfSphere(position - spherePosition4, 4.f); //Sphere SDF
    

    return smoothUnion(sphere4, smoothUnion(sphere3, smoothUnion(sphere1, sphere2)));
    //---------------------------------------------------------------------------------------------
    */
    /*
    //3rd BIT TO SHOW------------------------------------------------------------------------------
    float3 spherePosition1 = float3(20, cosine + 20, 60); //Position of the sphere in SDF Object World
    float sphere1 = sdfSphere(position - spherePosition1, 6.f); //Sphere SDF

    float3 spherePosition2 = float3(sine + 20.f, 10, 60); //Position of the sphere in SDF Object World
    float sphere2 = sdfSphere(position - spherePosition2, 6.f); //Sphere SDF

    float3 spherePosition3 = float3(sine + 30.0f, cosine + 10, 60); //Position of the sphere in SDF Object World
    float sphere3 = sdfSphere(position - spherePosition3, 6.f); //Sphere SDF

    float3 spherePosition4 = float3(sine + 10.f, 20, 60); //Position of the sphere in SDF Object World
    float sphere4 = sdfSphere(position - spherePosition4, 6.f); //Sphere SDF

    return smoothUnion(sphere4, smoothUnion(sphere3, smoothUnion(sphere1, sphere2)));
    //---------------------------------------------------------------------------------------------
    */
    /*
    //4th BIT TO SHOW------------------------------------------------------------------------------
    float3 spherePosition1 = float3(sine * 1.2 + 20, abs(cosine) + 7, 65); //Position of the sphere in SDF Object World
    float sphere1 = sdfSphere(position - spherePosition1, 5.f); //Sphere SDF

    float3 spherePosition2 = float3(-sine * 2.8 + 20, abs(cosine) + 13, 63); //Position of the sphere in SDF Object World
    float sphere2 = sdfSphere(position - spherePosition2, 5.f); //Sphere SDF

    float3 spherePosition3 = float3(sine * 2.4 + 20, abs(cosine) + 15, 60); //Position of the sphere in SDF Object World
    float sphere3 = sdfSphere(position - spherePosition3, 5.f); //Sphere SDF

    float3 spherePosition4 = float3(-sine * 1.3 + 20, abs(cosine) + 11, 61); //Position of the sphere in SDF Object World
    float sphere4 = sdfSphere(position - spherePosition4, 5.f); //Sphere SDF

    float3 spherePosition5 = float3(sine * 5.0f + 20, abs(cosine) + 5, 57); //Position of the sphere in SDF Object World
    float sphere5 = sdfSphere(position - spherePosition5, 5.f); //Sphere SDF

    float3 spherePosition6 = float3(-sine + 20, abs(cosine) + 10, 59); //Position of the sphere in SDF Object World
    float sphere6 = sdfSphere(position - spherePosition6, 5.f); //Sphere SDF

    float3 spherePosition7 = float3(sine + 20, abs(cosine) + 10, 53); //Position of the sphere in SDF Object World
    float sphere7 = sdfSphere(position - spherePosition7, 5.f); //Sphere SDF

    float3 spherePosition8 = float3(-sine + 20, abs(cosine) + 10, 55); //Position of the sphere in SDF Object World
    float sphere8 = sdfSphere(position - spherePosition8, 5.f); //Sphere SDF

    return smoothUnion(sphere8, smoothUnion(sphere7, smoothUnion(sphere6, smoothUnion(sphere5, smoothUnion(sphere4, smoothUnion(sphere3, smoothUnion(sphere1, sphere2)))))));
    //---------------------------------------------------------------------------------------------
    */
    
}


float4 main(InputType input) : SV_TARGET
{
    //Setting up colours for SDF
    float3 finalColour = float3(0,0,0);
    float4 posColour = float4(0.0, 0.0, 1.0f, 1.0f);
    float4 negColour = float4(1.0, 0.0, 0.0, 1.0f);

    //Initialising variables used for raymarching
    float3 rayOrigin = float3(0,0,-10.f);
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
        finalColour = float3(totalDistanceTravelled, totalDistanceTravelled, totalDistanceTravelled) /100;

        if (distanceToScene < 0.001f || totalDistanceTravelled > 100.f)//If the distance to an SDF shape becomes smaller than 0.001 stop iterating //Stop iterating if the ray moves too far without hitting any objects
        {
            break;
        }
           
        
        
    }

   


    //float distanceToSphere = sdfSphere(cameraPos, input.position.xyz, 1);
    
    //finalColour = distanceToSphere > 0.0f ? posColour : negColour;

	//finalColour = finalColour * exp(distanceToSphere);

	//return finalColour;

    return float4(1-finalColour,1.0f);
}