
Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture : register(t1);
Texture2D depthMapTexture2 : register(t2);
SamplerState diffuseSampler  : register(s0);
SamplerState shadowSampler : register(s1);

cbuffer LightBuffer : register(b0)    //Buffer in register 0, reused for multiple shaders, buffer data must match  
{
    float4 ambient;
	float4 dirDiffuse;
	float3 dirDirection;
    float padding;

    float spotAngle;
    float3 spotPos;
    float4 spotDiff;
    float3 spotDir;
    float padding2;
    
    float3 pointPos;
    float padding3;
    float4 pointDiff;
};

struct InputType        
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 dirLightPos : TEXCOORD1;
    float4 spotLightPosIn : TEXCOORD2;
    float3 worldPosition : TEXCOORD3;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, normalize(lightDirection)));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calculateSpotLight(float3 spotDir, float3 spotVec, float distance, float3 normal, float4 diffuse)        //Calculate spotlight intensity, attenuation is applied too
{
    float intensity = saturate(dot(normal, spotVec));  //gets the strenght of the illumination through the dot product between the normal and spotlightVector and saturate it(clamp between 0-1)
    float dotP = dot(spotDir, spotVec);                //dot product returns a value(scalar) to understand where the light lies

    
    float constantAttenuation, linearAttenuation, quadraticAttenuation;         //Attenuation values
    constantAttenuation = 0.0;
    linearAttenuation = 0.05;
    quadraticAttenuation = 0.0f;
    
    float4 colour = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (dotP > spotAngle)           //If the dot product is greater than the spotangle passed from the app1, then there is light
    {
        colour += saturate(diffuse * intensity);        //Combine the diffuse colour with the intensity
        float att = 1.0f / ((1 + linearAttenuation * distance) * (1 + quadraticAttenuation * distance * distance));     //Apply the attenuation 
        return saturate(colour * att);      //Return the colour of the spotlight
    }
    return colour;      //if not in the angle of the spotlight, the spotlight doesn't return a colour
}

// Is the geometry in our shadow map
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
float getHeight(float2 tex)     //Gets height for the normal mapping calculation
{
    float textureColour;
    textureColour = shaderTexture.SampleLevel(shadowSampler, tex, 0);
    return textureColour * 40;
}

float3 calcNormal(float2 uv)
{
    float texelSize ;       //Size of texture
    float val;

    shaderTexture.GetDimensions(0, texelSize, texelSize, val);             //Get dimension of texture and store the values in texelsize and val
    float uvOff = 5.0f / texelSize;     //How far the neighbour pixel
    float worldStep = 40.0f * uvOff;

    float heightN = getHeight(float2(uv.x, uv.y + uvOff));      //Calculate the different values for each direction from the pixel, moving in the texture with the uvOffset
    float heightS = getHeight(float2(uv.x, uv.y - uvOff));
    float heightE = getHeight(float2(uv.x + uvOff, uv.y));
    float heightW = getHeight(float2(uv.x - uvOff, uv.y));

    float3 tan = normalize(float3(2.0f * worldStep, heightE - heightW, 0));     //Measure the difference of elevation of neighbour pixels on u axis (x) for tangent and v axis (y) for bitangent
    float3 bitan = normalize(float3(0.0f, heightN - heightS, 2.0f * worldStep));
    return cross(bitan, tan);       //The normals are then obtained by the cross product between the bitangent and tangent.

}

bool isInShadow2(Texture2D sMap, float2 uv, float4 light2ViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = light2ViewPosition.z / light2ViewPosition.w;
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

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f;
    
    float4 dirColour = float4(0.f, 0.f, 0.f, 1.f);         // Lights initialization;
    float4 spotColour = float4(0.f, 0.f, 0.f, 1.f);        // Lights initialization;
    float4 pointColour = float4(1.f, 0.f, 0.f, 1.f);       // Lights initialization;
    
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
    float3 spotLightvector = normalize(spotPos - input.worldPosition);   //Vector from the terrain to the spotlight   
    float distance = length(spotPos - input.worldPosition);         //Distance between spotlight and terrain
    
	// Calculate the projected texture coordinates.
    float2 pTexCoord = getProjectiveCoords(input.dirLightPos);
    float2 pTexCoord2 = getProjectiveCoords(input.spotLightPosIn);
//    float2 pTexCoord2 = getProjectiveCoords(input.light2ViewPos);
    // Shadow test. Is or isn't in shadow
    if (padding == 1)
    {
        input.normal = calcNormal(input.tex);       //Calculate normals for manipulation
        float3 pointLightVector = normalize(pointPos - input.worldPosition);        //Pointlight vector from world to pointlight
        pointColour = calculateLighting(pointLightVector, input.normal, pointDiff); //Calculate colour of point light
        if (hasDepthData(pTexCoord))         //If the texture coordinates have depth data for the directional light
        {
        // Has depth map data
            if (!isInShadow(depthMapTexture, pTexCoord, input.dirLightPos, shadowMapBias))
            {
            // is NOT in shadow, therefore light
                dirColour = calculateLighting(-dirDirection, input.normal, dirDiffuse);
            }
        }        
        if (hasDepthData(pTexCoord2))        //If the texture coordinates have depth data for the spot light
        {
            if (!isInShadow2(depthMapTexture2, pTexCoord2, input.spotLightPosIn, shadowMapBias))
            {
                spotColour = calculateSpotLight(-spotDir, spotLightvector, distance, input.normal, spotDiff);
            }
        }
        float3 temp = input.normal;
        dirColour = (dirColour + ambient);
       // return pointColour;
       return (spotColour + dirColour+ pointColour) * textureColour;
       //return float4(temp, 1.0f);
    }
    else            //If not manipulated:
    {
        float3 pointLightVector = normalize(pointPos - input.worldPosition);            //Vector from world to pointlight
        pointColour = calculateLighting(pointLightVector, input.normal, pointDiff);     //calculate pointLight colour
        if (hasDepthData(pTexCoord))        //If the texture coordinates have depth data for the directional light
        {
        // Has depth map data
            if (!isInShadow(depthMapTexture, pTexCoord, input.dirLightPos, shadowMapBias))
            {
            // is NOT in shadow, therefore light
                dirColour = calculateLighting(-dirDirection, input.normal, dirDiffuse);
            }
        }
        if (hasDepthData(pTexCoord2))       //If the texture coordinates have depth data for the spotlight light
        {
            if (!isInShadow2(depthMapTexture2, pTexCoord2, input.spotLightPosIn, shadowMapBias))
            {
            // is NOT in shadow, therefore light
                spotColour = calculateSpotLight(-spotDir, spotLightvector, distance, input.normal, spotDiff);

            }
        }       
        return (spotColour + dirColour+pointColour) * textureColour;            //Return the lights combined and the texture (times so that the values don't go beyond 1)
    }
}
    