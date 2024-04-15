
// Texture and sampler registers
Texture2D texture0 : register(t0);
StructuredBuffer<float4> sdfParticlePositions : register(t1);

struct ParticleData
{
    int particleNum;
    float3 position;
    float3 predictedPosition;
    float3 velocity;
    float2 density;
    uint3 spatialIndices; //x is the original index //y is the hash //z is the key
    uint spatialOffsets;
};

//StructuredBuffer<ParticleData> sdfParticlePositions : register(t1);

SamplerState Sampler0 : register(s0);

cbuffer CameraBuffer : register(b0){
    float4 cameraPos;
    float4 timer;
}
cbuffer SDFBuffer : register (b1){
    float4 blendAmount;
    float4 numParticles;
}


struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 worldPosition : TEXCOORD1;
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
    
    float finalValue;

    float sphere1 = sdfSphere(position - float3(sdfParticlePositions[0].xyz), 1.f); //Sphere SDF
    float sphere2 = sdfSphere(position - float3(sdfParticlePositions[1].xyz), 1.f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

	if(numParticles.x>2){
	    for (int i = 2; i < numParticles.x; i++)
	    {
	        float sphere = sdfSphere(position - float3(sdfParticlePositions[i].xyz), 1.f); //Sphere SDF

    		finalValue = smoothUnion(sphere, finalValue);

	    }
    }
  
    return finalValue;
   
    
}


float4 main(InputType input) : SV_TARGET
{
    //Setting up colours for SDF
    float3 finalColour = float3(0,0,0);
    float4 posColour = float4(0.0, 0.0, 1.0f, 1.0f);
    float4 negColour = float4(1.0, 0.0, 0.0, 1.0f);

    //Initialising variables used for raymarching
    float3 rayOrigin = float3(0,-10,10.f);
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

   	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 textureColor = texture0.Sample(Sampler0, input.tex);


    //float distanceToSphere = sdfSphere(cameraPos, input.position.xyz, 1);
    
    //finalColour = distanceToSphere > 0.0f ? posColour : negColour;

	//finalColour = finalColour * exp(distanceToSphere);

    input.position = float4(input.tex.x, 0.f, input.tex.y, 0.f);

    //return float4(1 - finalColour, 1.0f) * textureColor;
    return float4(1 - finalColour, 1.0f);

}