// Terrain Manipulation vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

Texture2D mapHeight : register(t0);
Texture2D cliff : register(t1);
Texture2D moss : register(t2);
Texture2D snow : register(t3);
SamplerState sampler0 : register(s0);
SamplerState textureSampler : register(s1);


cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
    matrix spotlightViewMatrix;
    matrix spotlightProjectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
    float4 cameraPosition;
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos1 : TEXCOORD4;
    float4 lightViewPos2 : TEXCOORD5;
};


float getHeightMapTextureHeight(float2 uv)
{
    //Returns height from texture at a point based on colour
    float height;
    height = mapHeight.SampleLevel(sampler0, uv, 0).r;
    return height * 20;
}

float getMaterialTextureHeight(float2 uv, float3 normal,float3 position)
{
    //Returns height at a point based on texture used at a certain vertex
    float height;
    //If the normal's x or z component are bigger than 0.5 or smaller than -0.5 sample the heightmap texture of the rock/cliff
    if (normal.x > 0.5 || normal.x < -0.5 || normal.z>0.5||normal.z<-0.5)
    {
        height = cliff.SampleLevel(textureSampler, uv, 0).r;
    }
    else
    {
        //If the height of the vertex is above 20 sample the heightmap texture of the snow
        if (position.y>20.f)
        {
            height = snow.SampleLevel(textureSampler, uv, 0).r;
        }
        //In any other remaining position sample the heightmap texture of the grass/moss
        else{
            
            height = moss.SampleLevel(textureSampler, uv, 0).r;
        }
    }
    return height * 2;
}

float3 findNormals(InputType inputVal)
{
    float3 newNormal;
    
	
	//Offset we will calculate uvs with
    float offset = 1.f / 100.f; //its 100 because thats the default resolution of the plane
	
	//Find the vectors surrounding the current point
    float3 east = float3(inputVal.position.x + 0.5, getHeightMapTextureHeight(float2(inputVal.tex.x + offset, inputVal.tex.y)) + getMaterialTextureHeight(float2(inputVal.tex.x * 2.0f + offset, inputVal.tex.y * 2.0f),inputVal.normal,inputVal.position.xyz), inputVal.position.z);
    float3 west = float3(inputVal.position.x - 0.5, getHeightMapTextureHeight(float2(inputVal.tex.x - offset, inputVal.tex.y)) + getMaterialTextureHeight(float2(inputVal.tex.x * 2.0f - offset, inputVal.tex.y * 2.0f), inputVal.normal, inputVal.position.xyz), inputVal.position.z);
    float3 north = float3(inputVal.position.x, getHeightMapTextureHeight(float2(inputVal.tex.x, inputVal.tex.y + offset)) + getMaterialTextureHeight(float2(inputVal.tex.x * 2.0f, inputVal.tex.y * 2.0f + offset), inputVal.normal, inputVal.position.xyz), inputVal.position.z + 0.5);
    float3 south = float3(inputVal.position.x, getHeightMapTextureHeight(float2(inputVal.tex.x, inputVal.tex.y - offset)) + getMaterialTextureHeight(float2(inputVal.tex.x * 2.0f, inputVal.tex.y * 2.0f - offset), inputVal.normal, inputVal.position.xyz), inputVal.position.z - 0.5);
	
	//Calculate vector from north to south and from east to west
    float3 tangentWE = normalize(east - west);
    float3 bitangentNS = normalize(south - north);
	
	//Calculate the crossproduct of the vectors to find the normal at the point
    newNormal = cross(tangentWE, bitangentNS);
	
    return newNormal;
}

OutputType main(InputType input)
{
    OutputType output;
	
    //Increase plane size
    input.position.x *= 2.f;
    input.position.z *= 2.f;
	//Changing height of plane based on texture values
    input.position.y = (getHeightMapTextureHeight(input.tex) + getMaterialTextureHeight(input.tex, input.normal, input.position.xyz)) * 2.f;
	//Calculate new normals
    float3 newNormal = findNormals(input);
	

	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
    output.normal = mul(newNormal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    
    // Calculate the position of the vertice as viewed by the light source.
    output.lightViewPos1 = mul(input.position, worldMatrix);
    output.lightViewPos1 = mul(output.lightViewPos1, lightViewMatrix);
    output.lightViewPos1 = mul(output.lightViewPos1, lightProjectionMatrix);
    
    // Calculate the position of the vertice as viewed by the light source.
    output.lightViewPos2 = mul(input.position, worldMatrix);
    output.lightViewPos2 = mul(output.lightViewPos2, spotlightViewMatrix);
    output.lightViewPos2 = mul(output.lightViewPos2, spotlightProjectionMatrix);
    
    output.worldPosition = mul(input.position, worldMatrix).xyz;
	
    output.viewVector = normalize(cameraPosition.xyz - output.worldPosition.xyz);

    return output;
}
