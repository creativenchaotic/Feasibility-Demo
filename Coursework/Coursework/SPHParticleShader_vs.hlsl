// SPH Particle vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader
struct Particle
{
    int particleNum;
    float3 position;
    float3 predictedPosition;
    float3 velocity;
    float2 density;
    uint3 spatialIndices;
    uint spatialOffsets;
};

StructuredBuffer<Particle> particleData : register(t0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ParticleIndexBuffer : register(b1)
{
    int particleIndex;
    float3 particleIndexBufferPadding;
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
};


OutputType main(InputType input)
{
    OutputType output;
	
    input.position += float4(particleData[particleIndex].position, 0.0f);
	
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

    return output;
}
