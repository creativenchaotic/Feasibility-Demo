// Water vertex shader
//Includes vertex manipulations

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);


cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//Wave manipulation values
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
    
    float waterHeight;
    
    float4 direction3;
    
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
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float waterDepth : TEXCOORD2;
    float3 mountainWorldPos : TEXCOORD3;
};


InputType wavePlane(InputType inputVal)
{
	//Changing the height of the water plane
    float wave1Pos = 2 * amplitude1 * pow((((sin((dot(direction1.xz, float2(inputVal.position.x,inputVal.position.z))) * frequency1 + (time * (speed1 * frequency1)))) + 1) / 2), steepnessFactor);
    float wave2Pos = 2 * amplitude2 * pow((((sin((dot(direction2.xz, float2(inputVal.position.x, inputVal.position.z))) * frequency2 + (time * (speed2 * frequency2)))) + 1) / 2), steepnessFactor);
    float wave3Pos = 2 * amplitude3 * pow((((sin((dot(direction3.xz, float2(inputVal.position.x, inputVal.position.z))) * frequency3 + (time * (speed3 * frequency3)))) + 1) / 2), steepnessFactor);
 
    inputVal.position.y = wave1Pos + wave2Pos + wave3Pos;
    
    return inputVal;
}



float getWaveOffset(float x, float z)
{
    //Changing the height of the water plane
    float wave1Pos = 2 * amplitude1 * pow((sin(dot(direction1.xz, float2(x, z)) * frequency1 + (time * speed1 * frequency1)) + 1) / 2, steepnessFactor);
    float wave2Pos = 2 * amplitude2 * pow((sin(dot(direction2.xz, float2(x, z)) * frequency2 + (time * speed2 * frequency2)) + 1) / 2, steepnessFactor);
    float wave3Pos = 2 * amplitude3 * pow((sin(dot(direction3.xz, float2(x, z)) * frequency3 + (time * speed3 * frequency3)) + 1) / 2, steepnessFactor);
 
    float yPos = wave1Pos + wave2Pos + wave3Pos;
    
    return yPos;
}

float3 findNormals(InputType inputVal)
{
    float3 newNormal;
	
	//Offset we will calculate uvs with
    float offset = 1.f / 200.f; //its 100 because thats the default resolution of the plane * 2
	
	//Find the vectors surrounding the current point
    float3 east = float3(inputVal.position.x + offset, getWaveOffset(inputVal.position.x + offset, inputVal.position.z), inputVal.position.z);
    float3 west = float3(inputVal.position.x - offset, getWaveOffset(inputVal.position.x - offset, inputVal.position.z), inputVal.position.z);
    float3 north = float3(inputVal.position.x, getWaveOffset(inputVal.position.x, inputVal.position.z + offset), inputVal.position.z + offset);
    float3 south = float3(inputVal.position.x, getWaveOffset(inputVal.position.x, inputVal.position.z - offset), inputVal.position.z - offset);
	
	//Calculate vector from north to south and from east to west
    float3 tangentWE = normalize(east - west);
    float3 bitangentNS = normalize(south - north);
	
	//Calculate the crossproduct of the vectors to find the normal at the point
    newNormal = cross(tangentWE, bitangentNS);
	
    return newNormal;
}

float getHeightMapTextureHeight(float2 uv)
{
    //Returns height from texture at a texture based on colour
    float height;
    height = texture0.SampleLevel(sampler0, uv, 0).r;
    return height * 20;
}

//Calculating water depth
float waterDepth(InputType inputVal)
{
    float depth;
    
    depth = (getHeightMapTextureHeight(inputVal.tex) - (getWaveOffset(inputVal.position.x, inputVal.position.z) + waterHeight) + 4);
    
    depth = depth / (amplitude1 + amplitude2 + amplitude3 + 1);
    
    return depth;
}

OutputType main(InputType input)
{
    OutputType output;
	
    //Increase plane size
    input.position.x *= 2.f;
    input.position.z *= 2.f;
    input.position.y *= 2.f;

    //Calculating wave height
    input = wavePlane(input);
    input.position.y += waterHeight;

    //Finding new wave normals
    input.normal = findNormals(input);
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    
    //World position
    output.worldPos = mul(input.position, worldMatrix).xyz;
    
    //Getting the world position of the terrain
    output.mountainWorldPos = mul(float4(input.position.x, getHeightMapTextureHeight(input.tex),input.position.z,input.position.w), worldMatrix).xyz;
    
    //Water depth
    output.waterDepth = waterDepth(input);

    return output;
}
