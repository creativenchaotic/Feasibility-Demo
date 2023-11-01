//Shader used to get the depth map texture using the heightmap from the terrain
Texture2D heightmap : register(t0);
SamplerState heightmapSampler : register(s0);


cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};


float getHeightMapTextureHeight(float2 uv)
{
    //Returns height from texture at a texture based on colour
    float height;
    height = heightmap.SampleLevel(heightmapSampler, uv, 0).r;
    return height * 20;
}

OutputType main(InputType input)
{
    OutputType output;

    //Adding the heightmap displacement to input.position
    input.position.y += getHeightMapTextureHeight(input.tex);
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}