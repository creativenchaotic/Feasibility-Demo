// SPH Particle pixel shader
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

    //TODO: Implement PBR or another lighting technique so that you can actually tell whats going on with the particles
    
    finalColour = float4(0.46f, 0.89f, 1.0f, 0.8f);

    return finalColour;
}


