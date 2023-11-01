// Terrain Manipulation pixel shader
// Calculate PBR lighting and shadows

Texture2D mapHeight : register(t0);
Texture2D cliff : register(t1);
Texture2D moss : register(t2);
Texture2D snow : register(t3);
Texture2D cliffRoughness : register(t4);
Texture2D mossRoughness : register(t5);
Texture2D snowRoughness : register(t6);
Texture2D depthTexture : register(t7);
Texture2D depthTexture2 : register(t8);

SamplerState sampler0 : register(s0);
SamplerState textureSampler : register(s1);
SamplerState shadowSampler : register(s2);

//Light values
cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour[2];
    float4 ambientColour[2];
    float4 lightDirection[2];
    float4 lightPosition[2];
    
    float4 spotlightSize;
    float lightType[4];
};

//Attenuation Values
cbuffer AttenuationBuffer : register(b1)
{
    float constantFactor;
    float linearFactor;
    float quadraticFactor;
    float padding2;
}

//Camera Position
cbuffer CameraBuffer : register(b2)
{
    float4 cameraPos;
}

//Material Values
cbuffer MaterialBuffer : register(b3)
{
    float4 grassMetallic;
    float4 grassBaseReflectivity;
    float4 rockMetallic;
    float4 rockReflectivity;
    float4 snowMetallic;
    float4 snowReflectivity;
    float4 sandMetallic;
    float4 sandReflectivity;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos1 : TEXCOORD4;
    float4 lightViewPos2 : TEXCOORD5;
};

//FIND WHAT TEXTURE GETS USED BASED ON SLOPE---------------------------------------------------------------------------------------------------------------
float4 calculateTexture(InputType input)
{
    float4 finalTexture;
    
    //Rock Texture
    if (input.normal.x > 0.5 || input.normal.x < -0.5 || input.normal.z > 0.8 || input.normal.z < -0.8)
    {
        finalTexture = cliff.Sample(textureSampler, input.tex);
    }
   
    else
    {
        //Snow Texture
        if (input.worldPosition.y>20.f)
        {
            finalTexture = snow.Sample(textureSampler, input.tex);
        }
        //MossT Texture
        else
        {
            finalTexture = moss.Sample(textureSampler, input.tex);
        }
    }
    
    return finalTexture;
}

//ATTENUATION--------------------------------------------------------------------------------------------------------------------------------------
float4 attenuate(float3 lightVec, float4 diffuseLight)
{
    float distance = length(lightVec);
    
    //Find attenuation using attenuation equation
    float attenuation = 1 / (constantFactor + (linearFactor * distance) + (quadraticFactor * pow(distance, 2)));
    
    return diffuseLight * attenuation;

}

//LIGHTING CALCULATIONS------------------------------------------------------------------------------------------------------------------------------------

//Calculating Fresnel factor
float3 fresnel(float3 baseReflect, float3 viewVector, float3 halfwayVector, float4 txtrCol, float metallicFactor)
{
    //Base reflectivity based on base reflectivity getting passed in per material, the texture colour and the metallic property of the material
    baseReflect = lerp(baseReflect, txtrCol.xyz, metallicFactor);
    return baseReflect + (float3(1.0, 1.f, 1.f) - baseReflect) * pow(1 - max(dot(viewVector, halfwayVector), 0.0), 5.0);
}

//GGX/Trowbridge-Reitz normal distribution
float3 normalDistribution(float rough, float3 normal, float3 halfway)
{
    float pi = 3.141592653589793;
    
    float numerator = pow(rough, 2.0f);
    
    float NdotH = max(dot(normal, halfway), 0.0f);
    
    float denominator = pi * pow(pow(NdotH, 2.0) * (pow(rough, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.0000001);
    
    return float3(numerator / denominator, numerator / denominator, numerator / denominator);

}

//Schlick-Beckmann Geometric Shadow-Masking
float schlickbeckmannGS(float rough, float3 normal, float3 v)
{
    float numerator = max(dot(normal, v), 0.0f);
    
    float k = rough / 2.f;
    float denominator = max(dot(normal, v), 0.000001f) * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);
    
    return numerator / denominator;

}

//Smith Geometric Shadow-Masking
float geometryShadowing(float rough, float3 normal, float3 viewVector, float3 lightVector)
{
    return schlickbeckmannGS(rough, normal, viewVector) * schlickbeckmannGS(rough, normal, lightVector);
}

//Main PBR function
float3 PBR(InputType input, float3 cameraPosition, float reflectivity, float4 txtrColour, float roughVal, float metallicFactor, float type, float4 lightDiffuse, float3 lightDir, float4 lightAmbient)
{

    float3 finalLighting;
    
    //Calculate vectors
    float3 normal = normalize(input.normal);
    float3 viewVector = normalize(cameraPosition - input.worldPosition);
    float3 lightVector = normalize(-lightDir); //Directional light
    float3 halfwayVector = normalize(viewVector + lightVector);

    //Fresnel Factor (Ks) - Ks+Kd will always =1 due to energy conservation
    float3 Ks = fresnel(reflectivity, viewVector, halfwayVector, txtrColour, metallicFactor);
    float3 Kd = float3(1.0, 1.0, 1.0) - Ks;
    
    //Lambert used for fDiffuse
    float3 diffuse = txtrColour.xyz / 3.14f;
    
    //If the light is a point light or a spotlight
    if (type == 0.f)
    {
        lightDiffuse =  attenuate(lightVector, lightDiffuse);
    }
    
    //fSpecular
    float3 specularNumerator = normalDistribution(roughVal, normal, halfwayVector) * geometryShadowing(roughVal, normal, viewVector, lightVector) * fresnel(reflectivity, viewVector, halfwayVector, txtrColour, metallicFactor);
    float3 specularDenominator = 4.0 * max(dot(viewVector, normal), 0.0f) * max(dot(lightVector, normal), 0.0);
    specularDenominator = max(specularDenominator, 0.0000001);
    float3 specular = specularNumerator / specularDenominator;
    
    //Final BRDF calculation
    float3 BRDF = Kd * diffuse + specular;
    
    //Ambient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * txtrColour.xyz;
    
    //Final lighting
    finalLighting = float3(BRDF * lightDiffuse.xyz * max(dot(lightVector, normal), 0.0f)) + ambient;
    
    //HDR tonemapping
    finalLighting = finalLighting / (finalLighting + float3(1.0, 1.0, 1.0));
    
    //Gamma Correct
    finalLighting = pow(finalLighting, float3(1.0 / 2.5, 1.0 / 2.5, 1.0 / 2.5));
    
    return finalLighting;
}

//Passing in materials for different textures depending on slope
float3 calcPBRBasedOnMaterial(InputType input,float4 textureColour, float roughness, float type, float4 lightDiffuse, float3 lightDir, float4 lightAmbient)
{
    float3 finalLighting;
           
    //Rock
    if (input.normal.x > 0.5 || input.normal.x < -0.5 || input.normal.z > 0.8 || input.normal.z < -0.8)
    {
        finalLighting = PBR(input, cameraPos.xyz, rockReflectivity.x, textureColour, roughness, rockMetallic.x, type, lightDiffuse, lightDir, lightAmbient);

    }
    else
    {
        //Snow
        if (input.worldPosition.y > 20.f)
        {
            finalLighting = PBR(input, cameraPos.xyz, snowReflectivity.x, textureColour, roughness, snowMetallic.x, type, lightDiffuse, lightDir, lightAmbient);
        }
        
        //Moss
        else
        {
            finalLighting = PBR(input, cameraPos.xyz, grassBaseReflectivity.x, textureColour, roughness, grassMetallic.x, type, lightDiffuse, lightDir, lightAmbient);
        }
    }
    
    return saturate(finalLighting);
}

//Passing in different roughness maps per material depending on slope
float calcRoughnessBasedOnMaterial(InputType input)
{
    float roughnessIndex;
           
    //Rock
    if (input.normal.x > 0.5 || input.normal.x < -0.5 || input.normal.z > 0.8 || input.normal.z < -0.8)
    {
        roughnessIndex = cliffRoughness.SampleLevel(textureSampler, input.tex, 0).r - 0.5;

    }
    else
    {
        //Snow
        if (input.worldPosition.y > 20.f)
        {
            roughnessIndex = snowRoughness.SampleLevel(textureSampler, input.tex, 0).r - 0.5;
        }
        //Moss
        else
        {
            roughnessIndex = mossRoughness.SampleLevel(textureSampler, input.tex, 0).r - 0.5;
        }
    }
    
    return roughnessIndex;
}

//SPOTLIGHT PBR LIGHTING--------------------------------------------------------------------------------------------------------------------------------------
//This angle is half the angle of the spotlight
float4 spotlightLighting(float3 lightVec, float3 lightDir, float4 lightDiffuse, InputType input, float4 textureColour, float materialRoughness, float4 lightAmbient)
{
    float4 colour;

    //Finding angle between the vectors
    float spotlightAngle = dot(normalize(-lightVec), lightDir);
    
    //Ambient lighting
    float3 ambient = float3(0.3, 0.3, 0.3) * textureColour.xyz;
    
    //If the angle between the vectors is not larger than the spotlight value illuminate the fragment
    if (spotlightAngle > spotlightSize.x)
    {
        colour = float4(ambient, 1.0f) + float4(calcPBRBasedOnMaterial(input, textureColour, materialRoughness, 1.0f, lightDiffuse, lightDir, lightAmbient), 1.0f);
        
        float smoothSpotlight = smoothstep(0, cos(spotlightSize.x), spotlightAngle);
        
        return colour * smoothSpotlight;

    }
    

    //Otherwise return ambient lighting
    return float4(ambient,1.f);

}

//SHADOWS-----------------------------------------------------------------------------------------------------------------------------------------------------
// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}



//MAIN--------------------------------------------------------------------------------------------------------------------------------------------------------
float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float materialRoughness;
    
    // Finding the texture and roughness based on slope
    textureColour = calculateTexture(input);
    materialRoughness = calcRoughnessBasedOnMaterial(input);
    
    float shadowMapBias = 0.005f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);

	// Calculate the projected texture coordinates.
    float2 pTexCoord = getProjectiveCoords(input.lightViewPos1);
    float2 pTexCoord2 = getProjectiveCoords(input.lightViewPos2);
    
    float4 finalLight = float4(0.f,0.f,0.f,1.0f);
    
    for (int i = 0; i < 2; i++)
    {
        float3 lightVector = (lightPosition[i].xyz - input.worldPosition); //Vector from the light to the pixel thats getting lit
        
        float4 combinedLightColour = float4(0.f, 0.f, 0.f, 0.f);
        
        //If the light is not off
        if (!(diffuseColour[i].x == 0.f && diffuseColour[i].y == 0 && diffuseColour[i].z == 0))
        {
        
            // Shadow test. Is or isn't in shadow
            if (hasDepthData(pTexCoord))
            {
                // Has depth map data
                if (!isInShadow(depthTexture, pTexCoord, input.lightViewPos1, shadowMapBias))
                {
                    //Create a different type of light depending on the type chosen
                    //directional light
                    if (lightType[i] == -1.0f)
                    {
                        combinedLightColour = float4(calcPBRBasedOnMaterial(input, textureColour, materialRoughness, -1.0f, diffuseColour[i], lightDirection[i].xyz, ambientColour[i]), 1.0f);
                    }
                }
            }
            
            if (hasDepthData(pTexCoord2))
            {
                if (!isInShadow(depthTexture2, pTexCoord2, input.lightViewPos2, shadowMapBias))
                {
                    //spotlight
                    if (lightType[i] == 0.0f)
                    {
                        combinedLightColour = spotlightLighting(lightVector, lightDirection[i].xyz, diffuseColour[i], input, textureColour, materialRoughness, ambientColour[i]);

                    }
                }
            }

                combinedLightColour += saturate(float4(0.3 * textureColour.x, 0.3 * textureColour.y, 0.3 * textureColour.z, 1.0));
                 //combinedLightColour += saturate(float4(0,0,0, 1.0)); // Return balck shadow, used for DEBUG
        }
        
        finalLight += combinedLightColour;
        
    }
    
    return saturate(finalLight);
  

}


