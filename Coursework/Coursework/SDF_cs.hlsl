//compute shader used to create a 3D texture out of the SDF calculations

RWStructuredBuffer<float4> particleData : register(u0); //Data we pass to and from the compute shader. Currently the particle data, this should probably be an SRV instead since we are wanting to output the results of the SDF from this shader.
RWTexture3D<float> SDFImage : register(u1);


cbuffer cb_simConstants : register(b0){
    int numParticles;
    float blendAmount;
    int stride;
    float offset;
    float4 simType;
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

    float sphere1 = sdfSphere(position - ((float3(particleData[0].xyz))), 1.0f); //Sphere SDF
    float sphere2 = sdfSphere(position - ((float3(particleData[1].xyz))), 1.0f); //Sphere SDF

    finalValue = smoothUnion(sphere1, sphere2);

    if (numParticles.x > 2)
    {
        for (int i = 2; i < numParticles.x; i++)
        {
            float sphere = sdfSphere(position - ((float3(particleData[i].xyz))), 1.0f); //Sphere SDF

            finalValue = smoothUnion(sphere, finalValue);

        }
    }
  
    return finalValue;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID)
{
    if (simType.x == 1)
    {
        uint3 resolution;
        SDFImage.GetDimensions(resolution.x, resolution.y, resolution.z);

        float3 worldMin = float3(-offset, -offset, -offset);
        float3 worldMax = float3(offset, offset, offset);

        float3 position = lerp(worldMin, worldMax, DTid / (float3(resolution) - 1.0f)) * 0.5;

        float sdfCalc = sdfCalculations(position);
        SDFImage[DTid.xyz] = sdfCalc; // Assign the calculated SDF value to the corresponding texel
    }

}