// SPH Particle pixel shader
//Returns a simple white colour for the light

//Lighting values
cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float4 ambientColour;
    float4 lightDirection;
    float4 lightPosition;
};

//Camera position
cbuffer CameraBuffer : register(b1)
{
    float4 cameraPos;
    float4 renderSetting;
}

//Material Values
cbuffer MaterialBuffer : register(b2)
{
    float roughness;
    float metallic;
    float baseReflectivity;
    float padding;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
};


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
float3 PBR(InputType input, float roughness, float4 particleColour, float3 cameraPosition, float3 lightDir, float baseReflect, float metallicFactor, float4 lightDiffuse, float4 lightAmbient)
{
    //Values needed
    float3 lighting;
    float3 normal = normalize(input.normal);
    float3 viewVector = normalize(cameraPosition - input.worldPos);
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


float4 main(InputType input) : SV_TARGET
{
    //Particle Colour
    float4 finalColour;
    finalColour = float4(0.46f, 0.89f, 1.0f, 0.8f);
    
    //PBR
    float4 finalLight = float4(0.f, 0.f, 0.f, 1.0f);
    
    if (renderSetting.x == -1.f)
    {
        float3 lightVector = (lightPosition.xyz - input.worldPos); //Vector from the light to the pixel thats getting lit
        
        float4 combinedLightColour = float4(0.f, 0.f, 0.f, 0.f);
        
        if (!(diffuseColour.x == 0.f && diffuseColour.y == 0 && diffuseColour.z == 0))
        {
            //directional light
            combinedLightColour = saturate(float4(PBR(input, roughness, finalColour, cameraPos.xyz, lightDirection.xyz, baseReflectivity, metallic, diffuseColour, ambientColour), 1.0f));
        }
        
        finalLight += combinedLightColour;
    }
    

    //RENDERING BASED ON RENDER SETTINGS
    if (renderSetting.x == -1.f)
    {
        return float4(finalLight.xyz, finalColour.a);
    }
    else if (renderSetting.x == 0.f)
    {
        return float4(saturate(input.worldPos.xyz), 1.0f);

    }
    else if (renderSetting.x == 1.f)
    {
        return float4(input.normal.xyz, 1.0f);
    }
    else
    {
        return float4(finalLight.xyz, finalColour.a);
    }
}


