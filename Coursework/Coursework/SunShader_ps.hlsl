// Sun pixel shader
//Returns a simple white colour for the light

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{
    float4 finalColour;

    
    finalColour = float4(1.0f,1.0f,1.0f,1.0f);

    return finalColour;
}


