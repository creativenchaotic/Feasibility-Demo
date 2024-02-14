

cbuffer CameraBuffer : register(b0){
    float4 cameraPos;
}


struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float sdfSphere(float3 camera, float3 spherePos, float radius)
{
    return length(camera-spherePos) - radius;
}

float4 main(InputType input) : SV_TARGET
{
    //Setting up colours for SDF
    float4 finalColour;
    float4 posColour = float4(0.0, 0.0, 1.0f, 1.0f);
    float4 negColour = float4(1.0, 0.0, 0.0, 1.0f);

    float distanceToSphere = sdfSphere(cameraPos, input.position.xyz, 1);
    
    finalColour = distanceToSphere > 0.0f ? posColour : negColour;

	finalColour = finalColour * exp(distanceToSphere);

	return finalColour;

    //return float4(1.0f,1.0f,1.0f,1.0f);
}