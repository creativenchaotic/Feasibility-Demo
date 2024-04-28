

// Texture and sampler registers
//Texture2D texture0 : register(t0);
StructuredBuffer<float4> sdfParticlePositions : register(t1);
Texture3D<float> texture3d : register(t0);

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

SamplerState Sampler0 : register(s0);
SamplerState Sampler3D : register(s1);

cbuffer CameraBuffer : register(b0){
    float4 cameraPos;
    float4 timer;
    matrix viewMatrix;
}
cbuffer SDFBuffer : register (b1){
    float blendAmount;
    int numParticles;
    int renderSetting;
    float padding2;
}

//Material Values
cbuffer MaterialBuffer : register(b2)
{
    float roughness;
    float metallic;
    float baseReflectivity;
    float padding;
}

//Lighting values
cbuffer LightBuffer : register(b3)
{
    float4 diffuseColour;
    float4 ambientColour;
    float4 lightDirection;
    float4 lightPosition;
};

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

float3 calcNormal(float3 pos)
{
    float3 step = float3(0.001f, 0.0f, 0.0f);
    float3 normal;
    normal.x = sdfCalculations(pos + step.xyy) - sdfCalculations(pos - step.xyy);
    normal.y = sdfCalculations(pos + step.yxy) - sdfCalculations(pos - step.yxy);
    normal.z = sdfCalculations(pos + step.yyx) - sdfCalculations(pos - step.yyx);
    return normalize(normal);
}


//LIGHTING CALCULATIONS------------------------------------------------------
//Fresnel factor
float3 fresnel(float3 baseReflect, float3 viewVector, float3 halfwayVector, float4 waterColour, float metallicFactor)
{ //Base reflectivity based on base reflectivity getting passed in per material, the texture colour and the metallic property of the material
    baseReflect = lerp(baseReflect, waterColour.xyz, metallicFactor);
    return baseReflect + (float3(1.0, 1.f, 1.f) - baseReflect) * pow(1 - max(dot(viewVector, halfwayVector), 0.0), 5.0);
}

//GGX/Trowbridge-Reitz normal distribution
float3 normalDistribution(float roughness, float3 normal, float3 halfway)
{
    float pi = 3.141592653589793;
    
    float numerator = pow(roughness, 2.0f);
    
    float NdotH = max(dot(normal, halfway), 0.0f);
    
    float denominator = pi * pow(pow(NdotH, 2.0) * (pow(roughness, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.0000001);
    
    return float3(numerator / denominator, numerator / denominator, numerator / denominator);
}

//Schlick-Beckmann Geometric Shadow-Masking
float schlickbeckmannGS(float roughness, float3 normal, float3 v)
{
    float numerator = max(dot(normal, v), 0.0f);
    
    float k = roughness / 2.f;
    float denominator = max(dot(normal, v), 0.000001f) * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);
    
    return numerator / denominator;
}

//Smith Geometric Shadow-Masking
float geometryShadowing(float roughness, float3 normal, float3 viewVector, float3 lightVector)
{
    return schlickbeckmannGS(roughness, normal, viewVector) * schlickbeckmannGS(roughness, normal, lightVector);
}


//Main PBR function
float3 PBR(float3 worldPos, float3 normalVector, float roughness, float4 particleColour, float3 cameraPosition, float3 lightDir, float baseReflect, float metallicFactor, float4 lightDiffuse, float4 lightAmbient)
{
    //Values needed
    float3 lighting;
    float3 normal = normalize(normalVector);
    float3 viewVector = normalize(cameraPosition - worldPos);
    float3 lightVector = normalize(-lightDir); //Directional light
    float3 halfwayVector = normalize(viewVector + lightVector);

    //Fresnel Factor (Ks)
    float3 Ks = fresnel(baseReflect, viewVector, halfwayVector, particleColour, metallicFactor);
    float3 Kd = float3(1.0, 1.0, 1.0) - Ks;
    
    //Lambert used for fDiffuse
    float3 diffuse = particleColour.xyz / 3.14f;
    
    //fSpecular
    float3 specularNumerator = normalDistribution(roughness, normal, halfwayVector) * geometryShadowing(roughness, normal, viewVector, lightVector) * fresnel(baseReflect, viewVector, halfwayVector, particleColour, metallicFactor);
    float3 specularDenominator = 4.0 * max(dot(viewVector, normal), 0.0f) * max(dot(lightVector, normal), 0.0);
    specularDenominator = max(specularDenominator, 0.0000001);
    float3 specular = specularNumerator / specularDenominator;
    
    //BRDF Calculation
    float3 BRDF = Kd * diffuse + specular;
    
    //Ambient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * particleColour.xyz;
 
    //Final lighting
    lighting = float3(BRDF * lightDiffuse.xyz * max(dot(lightVector, normal), 0.0f)) + ambient;
    
    //HDR tonemapping
    lighting = lighting / (lighting + float3(1.0, 1.0, 1.0));
    
    //Gamma Correct
    lighting = pow(lighting, float3(1.0 / 2.5, 1.0 / 2.5, 1.0 / 2.5));
 
    
    return lighting;
}

float4 calcLighting(float3 worldPos, float3 normal, float4 particleColour)
{

    float4 finalLight = float4(0, 0, 0, 1);

    float3 lightVector = (lightPosition.xyz - worldPos); //Vector from the light to the pixel thats getting lit
        
    float4 combinedLightColour = float4(0.f, 0.f, 0.f, 0.f);
        
    if (!(diffuseColour.x == 0.f && diffuseColour.y == 0 && diffuseColour.z == 0))
    {
            //directional light
        combinedLightColour = saturate(float4(PBR(worldPos, normal, roughness, particleColour, cameraPos.xyz, lightDirection.xyz, baseReflectivity, metallic, diffuseColour, ambientColour), 1.0f));
    }
        
    finalLight += combinedLightColour;

    return finalLight;
}

//----------------------------------------------------------------------------------------------------------------

bool intersectionCheck(float3 ro, float3 rd, out float2 intersectionPoints)
{
    float3 pos = float3(0, 0, 0);
    float scale = 20.f;

    float3 boxTranslation = pos * scale;

    ro += boxTranslation;

    //Ray Box Intersection
    float3 bmin = float3(-20, -20, -20);
    float3 bmax = float3(scale, scale, scale);

    float3 ri = 1.f / rd;
    float3 tbot = ri * (bmin - ro);
    float3 ttop = ri * (bmax - ro);

    float tmin = max(max(min(ttop.x, tbot.x), min(ttop.y, tbot.y)), min(ttop.z, tbot.z));
    float tmax = min(min(max(ttop.x, tbot.x), max(ttop.y, tbot.y)), max(ttop.z, tbot.z));

    if (tmax < 0 || tmin > tmax)
    {
        return false;
    }

    intersectionPoints = float2(tmin, tmax);

    return true;
}

float4 main(InputType input) : SV_TARGET
{

    /*
      //Setting up colours for SDF
    float3 finalColour = float3(0, 0, 0);
    float4 posColour = float4(0.0, 0.0, 1.0f, 1.0f);
    float4 negColour = float4(1.0, 0.0, 0.0, 1.0f);

    //Initialising variables used for raymarching
    float3 rayOrigin = float3(0, -10, 10.f);
    //float3 rayOrigin = cameraPos;
    float3 rayDirection = normalize(float3(input.tex * 0.7f, 1)); //Sets the direction of the ray to each point in the plane based on UVs
    float totalDistanceTravelled = 0.f; //Total distance travelled by ray from the camera's position
    float3 pointOfIntersection;

    //Raymarching (Sphere tracing)

    for (int i = 0; i < 100; i++)//The number of steps affects the quality of the results and the performance
    {
        float3 positionInRay = rayOrigin + rayDirection * totalDistanceTravelled; //Current position along the ray based on the distance from the rays origin

        float distanceToScene = texture3d.Sample(Sampler3D, positionInRay); //Current distance to the scene. Safe distance the point can travel to in any direction without overstepping an object

        totalDistanceTravelled += distanceToScene;

        pointOfIntersection = rayOrigin + rayDirection * totalDistanceTravelled;

         //Colouring
        finalColour = float3(totalDistanceTravelled, totalDistanceTravelled, totalDistanceTravelled) / 100;

        if (distanceToScene < 0.001f || totalDistanceTravelled > 100.f)//If the distance to an SDF shape becomes smaller than 0.001 stop iterating //Stop iterating if the ray moves too far without hitting any objects
        {
            break;
        }

    }

       // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 textureColor = texture0.Sample(Sampler0, input.tex);

    input.position = float4(input.tex.x, 0.f, input.tex.y, 0.f);

    //return float4(1 - finalColour, 1.0f) * textureColor;
    return float4(1 - finalColour, 1.0f);*/

    //----------------------------------------------------------------------------------------------------------------

     //Setting up colours for SDF
    float3 finalColour = float3(0, 0, 0);
    float4 finalLight = float4(0, 0, 0, 1);
    float4 waterColour = float4(0.23, 0.56, 0.96f, 1.0f);

    //Initialising variables used for raymarching
    //float3 rayOrigin = float3(-0.5,0.5,-5);
    float3 rayOrigin = cameraPos;
    float aspectRatio = 675 * 1248;
    float3 rayDirection = normalize(float3((input.tex.x) * 2.0f - 0.5f, input.tex.y * 2.0f - 0.5f, 3));


    rayDirection = normalize(mul(float4(rayDirection.x, rayDirection.y, rayDirection.z, 1), viewMatrix).xyz);

    //float3 rayDirection = normalize(mul(float4(input.tex, 1, 1), viewMatrix)).xyz; //Sets the direction of the ray to each point in the plane based on UVs
    float totalDistanceTravelled = 0.f; //Total distance travelled by ray from the camera's position
    float3 positionInRay;
    float3 normal;
    float distanceToScene;
    float2 intersectionPoints;

    //bool res = intersectionCheck(rayOrigin, rayDirection, invRayDirection);
    bool res = intersectionCheck(rayOrigin, rayDirection, intersectionPoints);

    if(res)
    {
        //Raymarching (Sphere tracing)
        for (int i = 0; i < 100; i++)//The number of steps affects the quality of the results and the performance
        {
            positionInRay = rayOrigin + rayDirection * (totalDistanceTravelled + intersectionPoints.x); //Current position along the ray based on the distance from the rays origin
            normal = calcNormal(positionInRay);
            distanceToScene = texture3d.SampleLevel(Sampler3D, positionInRay / 20, 0) /20; //Current distance to the scene. Safe distance the point can travel to in any direction without overstepping an object

            totalDistanceTravelled += distanceToScene;

            if (distanceToScene < 0.01f)//If the distance to an SDF shape becomes smaller than 0.001 stop iterating //Stop iterating if the ray moves too far without hitting any objects
            {
                switch(renderSetting)
                {
	                case 0:
                        finalLight = calcLighting(positionInRay / 20, normal, waterColour);
                        return (float4(finalLight.xyz, 1.0f) * waterColour);
                        
	                case 1:
                        finalColour = float3(positionInRay.x, positionInRay.y, positionInRay.z);
                        return float4(finalColour, 1);
                        
	                case 2:
                        finalColour = float3(normal);
                        return float4(finalColour, 1);
	                case 3:
                        finalColour = float3(totalDistanceTravelled, totalDistanceTravelled, totalDistanceTravelled) / 100;
                        return float4(1-finalColour, 1) * waterColour * 0.4f / 0.2f;
	                    
                }

                break;
            }

        }

        if (renderSetting == 3)
        {
            return float4(1, 1, 0, 1);
        }
        if(renderSetting == 2 || renderSetting == 1)
        {
            return float4(0,0,0,1);
        }
        if(renderSetting == 0)
        {
            return float4(0,0,0,0);
        }

    }
    else
    {
        if (renderSetting == 1 || renderSetting == 2 || renderSetting == 3)
        {
            return float4(0, 0, 0, 1);
        }

        if(renderSetting == 0)
        {
            return float4(0,0,0,0);
        }
    }

    return float4(1,0,0,1);

}