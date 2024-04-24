//compute shader used for SDF Rendering

static const int NumThreads = 64;

RWStructuredBuffer<float4> particleData : register(u0); //Data we pass to and from the compute shader. Currently the particle data, this should probably be an SRV instead since we are wanting to output the results of the SDF from this shader.
RWTexture3D<snorm float> SDFImage : register(u1);


cbuffer cb_simConstants : register(b0){
    int numParticles;
    float blendAmount;
}


float sdfSphere(float3 position, float radius)
{
    //We return the length of the vector that describes the distance between the camera and the surface of the sphere
    //This would be the length of the camera vector - the position of the centre of the sphere - the radius of the sphere

    return length(position) - radius; //Got this from Inigo Quilez
}


float smoothUnion(float shapeA, float shapeB)
{
    float h = max(blendAmount - abs(shapeA - shapeB), 0.0f) / blendAmount;
    return min(shapeA, shapeB) - h * h * h * blendAmount * (1.0f / 6.0f);
}


float sdfCalculations(float3 position)
{
        
    float finalValue;

    float sphere1 = sdfSphere(position - float3(particleData[0].xyz), 1.f); //Sphere SDF
    float sphere2 = sdfSphere(position - float3(particleData[1].xyz), 1.f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles.x > 2)
    {
        for (int i = 2; i < numParticles.x; i++)
        {
            float sphere = sdfSphere(position - float3(particleData[i].xyz), 1.f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint3 groupId : SV_GroupID, uint3 GroupThreadID : SV_GroupThreadID, int groupIndex : SV_GroupIndex )
{
    uint3 resolution;
    SDFImage.GetDimensions(resolution.x, resolution.y, resolution.z);
}