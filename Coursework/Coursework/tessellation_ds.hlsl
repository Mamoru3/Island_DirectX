// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
Texture2D texture0 : register(t1);
SamplerState sampler0 : register(s1);

cbuffer MatrixBuffer : register(b0)     //Buffer with all the informations sent from the tessellationShader.cpp
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    
    matrix dirLightView;
    matrix dirLightProj;
    matrix spotView;
    matrix spotProj;
};
struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct InputType            //Input from hull shader
{
    float4 position : POSITION;
    float2 tex: TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;

};

struct OutputType           //Output to the shadow_ps
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 dirLightPos : TEXCOORD1;
    float4 spotLightPosIn : TEXCOORD2;
    float3 worldPosition : TEXCOORD3;


};
float getHeight(float2 tex)     //sets height of the height map based on the texture colours
{
    float textureColour;
    textureColour = texture0.SampleLevel(sampler0, tex, 0);
    return textureColour * 10;
}
[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
    float2 texPosition;
    OutputType output;
    // Determine the position of the new vertex.
  //-----------Obtain one point lerping from the first point and the second point in the y coordinate
    float3 v1 = lerp(patch[0].position, patch[1].position, uvwCoord.y);     //To obtain the vertex position we 
      //-----------Obtain one point lerping from the third point and the fourth point in the y coordinate
    float3 v2 = lerp(patch[3].position, patch[2].position, uvwCoord.y);
    //--------------Lerping between these two y points on the x axis, we get the point on the x axis
    vertexPosition = lerp(v1, v2, uvwCoord.x);
    
    
    //Same that was done for the position is done for the texture position
    float2 t1 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y);
    float2 t2 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y);
    texPosition = lerp(t1, t2, uvwCoord.x);
    //Apply the vertex manipulation to create the manipulated terrain
    vertexPosition.y += getHeight(texPosition);
    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.worldPosition = output.position; 
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Calculate the position of the vertice as viewed by the directional light.
    output.dirLightPos = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.dirLightPos = mul(output.dirLightPos, dirLightView);
    output.dirLightPos = mul(output.dirLightPos, dirLightProj);
    // Calculate the position of the vertice as viewed by the spotlight. 
    output.spotLightPosIn = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.spotLightPosIn = mul(output.spotLightPosIn, spotView);
    output.spotLightPosIn = mul(output.spotLightPosIn, spotProj);
   // Store the texture coordinates for the pixel shader.
    output.tex = texPosition;
    
    return output;
}

