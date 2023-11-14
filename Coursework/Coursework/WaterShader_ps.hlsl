// Water pixel shader
// Caculate PBR lighting and final water colour

//Texture2D texture0 : register(t0);
//SamplerState sampler0 : register(s0);

//Lighting values
cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour[2];
    float4 ambientColour[2];
    float4 lightDirection[2];
    float4 lightPosition[2];

    float4 spotlightSize;
    float lightType[4];
};

//Camera position
cbuffer CameraBuffer : register(b1)
{
    float4 cameraPos;
}

//Wave vertex manipulation values
cbuffer TimeBuffer : register(b2)
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

//Attenuation values
cbuffer AttenuationBuffer : register(b3)
{
    float constantFactor;
    float linearFactor;
    float quadraticFactor;
    float padding2;
}

//Material Values
cbuffer MaterialBuffer : register(b4)
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

//ATTENUATION--------------------------------------------------------------------------------------------------------------------------------------
float4 attenuate(float3 lightVec, float4 diffuseLight)
{
    float distance = length(lightVec);
    
    //Attenuation equation
    float attenuation = 1 / (constantFactor + (linearFactor * distance) + (quadraticFactor * pow(distance, 2)));
    
    return diffuseLight * attenuation;

}

//LIGHTING CALCULATIONS------------------------------------------------------
//Fresnel factor
float3 fresnel(float3 baseReflect, float3 viewVector, float3 halfwayVector, float4 waterColour, float metallicFactor)
{   //Base reflectivity based on base reflectivity getting passed in per material, the texture colour and the metallic property of the material
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
float3 PBR(InputType input, float roughness, float4 waterColour, float3 cameraPosition, float3 lightDir, float baseReflect, float metallicFactor, float type, float4 lightDiffuse, float4 lightAmbient)
{
    //Values needed
    float3 lighting;
    float3 normal = normalize(input.normal);
    float3 viewVector = normalize(cameraPosition - input.worldPos);
    float3 lightVector = normalize(-lightDir); //Directional light
    float3 halfwayVector = normalize(viewVector + lightVector);

    //Fresnel Factor (Ks)
    float3 Ks = fresnel(baseReflect, viewVector, halfwayVector, waterColour, metallicFactor);
    float3 Kd = float3(1.0, 1.0, 1.0) - Ks;
    
    //Lambert used for fDiffuse
    float3 diffuse = waterColour.xyz / 3.14f;
    
    //If the light is a point light or a spotlight
    if (type == 0.f)
    {
        lightDiffuse = attenuate(lightVector, lightDiffuse);
    }
    
    //fSpecular
    float3 specularNumerator = normalDistribution(roughness, normal, halfwayVector) * geometryShadowing(roughness, normal, viewVector, lightVector) * fresnel(baseReflect, viewVector, halfwayVector, waterColour, metallicFactor);
    float3 specularDenominator = 4.0 * max(dot(viewVector, normal), 0.0f) * max(dot(lightVector, normal), 0.0);
    specularDenominator = max(specularDenominator, 0.0000001);
    float3 specular = specularNumerator / specularDenominator;
    
    //BRDF Calculation
    float3 BRDF = Kd * diffuse + specular;
    
    //Ambient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * waterColour.xyz;
 
    //Final lighting
    lighting = float3(BRDF * lightDiffuse.xyz * max(dot(lightVector, normal), 0.0f)) + ambient;
    
    //HDR tonemapping
    lighting = lighting / (lighting + float3(1.0, 1.0, 1.0));
    
    //Gamma Correct
    lighting = pow(lighting, float3(1.0 / 2.5, 1.0 / 2.5, 1.0 / 2.5));
 
    
    return lighting;
}

//SPOTLIGHT--------------------------------------------------------------------------------------
float4 spotlightLighting(InputType input,float3 lightVec, float rough, float4 waterColour, float3 cameraPosition, float3 lightDir, float baseReflect, float metallicFactor, float type, float4 lightDiffuse, float4 lightAmbient)
{
    float4 colour;

    //Finding angle between the vectors
    float spotlightAngle = dot(normalize(-lightVec), lightDir);
    
    //Ambient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * waterColour.xyz;
    
     //If the angle between the vectors is not larger than the spotlight value illuminate the fragment
    if (spotlightAngle > spotlightSize.x)
    {
        colour = float4(ambient, 1.0f) + float4(PBR(input, rough, waterColour, cameraPosition, lightDir, baseReflect, metallicFactor, type, lightDiffuse, lightAmbient), 1.0f);
        
        float smoothSpotlight = smoothstep(0, cos(spotlightSize.x), spotlightAngle);
        
        return colour * smoothSpotlight;

    }
    

    
    return float4(ambient, 1.f);

}

float4 main(InputType input) : SV_TARGET
{
    
    //Calculating depth colour-------------------------------------
    float4 shallowWaterColour = float4(0.26f, 0.83f, 0.9f, 0.5f);
    float4 deepWaterColour = float4(0.f,0.678f,0.819f,1.0f);
	
    //float4 finalDepthColour = lerp(deepWaterColour, shallowWaterColour, input.waterDepth);
    
    
    //-------------------------------------------------------------------------------
    //No shadows on water because objects reflect off of water, they dont create shadows
    
    float4 finalLight = float4(0.f, 0.f, 0.f, 1.0f);
    
    for (int i = 0; i < 2; i++)
    {
        float3 lightVector = (lightPosition[i].xyz - input.worldPos); //Vector from the light to the pixel thats getting lit
        
        float4 combinedLightColour = float4(0.f, 0.f, 0.f, 0.f);
        
        if (!(diffuseColour[i].x == 0.f && diffuseColour[i].y == 0 && diffuseColour[i].z == 0))
        {
            //Create a different type of light depending on the type chosen
            //directional light
            if (lightType[i] == -1.0f)
            {
                combinedLightColour = saturate(float4(PBR(input, roughness, deepWaterColour, cameraPos.xyz, lightDirection[i].xyz, baseReflectivity, metallic, lightType[i], diffuseColour[i], ambientColour[i]), 1.0f));
            }
            //spotlight
            if (lightType[i] == 0.0f)
            {
                combinedLightColour = saturate(spotlightLighting(input, lightVector, roughness, deepWaterColour, cameraPos.xyz, lightDirection[i].xyz, baseReflectivity, metallic, lightType[i], diffuseColour[i], ambientColour[i]));

            }
            
            //combinedLightColour += saturate(float4(0.4 * finalDepthColour.x, 0.4 * finalDepthColour.y, 0.4 * finalDepthColour.z, 1.0));
        }
        
        finalLight += combinedLightColour;
        
    }
    
    return float4(finalLight.xyz, deepWaterColour.a);
    
}


