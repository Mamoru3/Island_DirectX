// texture vertex shader
// Basic shader for rendering textured geometry

cbuffer MatrixBuffer : register(b0)     //Buffer that holds the information sent from the waterShader.cpp in the 0 register
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    
    matrix dirView;
    matrix dirProj;
    
    matrix spotView;
    matrix spotProj;
};
cbuffer waterBuffer : register(b1)      //Buffer that holds the information sent from the waterShader.cpp in the 1 register
{
    float time;
    float padding3;
};
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType                       //Output to go to the pixelShader
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 dirLightPos : TEXCOORD1;
    float4 spotLightPosIn : TEXCOORD2;
    float3 worldPosition : TEXCOORD3;
};

OutputType main(InputType input)
{
    OutputType output;

    input.position.y = 0.5 * sin((input.position.x * 0.4) + time);      //Manipulation to recreate waves in the lake, uses time 

// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the directional light.
    output.dirLightPos = mul(input.position, worldMatrix);
    output.dirLightPos = mul(output.dirLightPos, dirView);
    output.dirLightPos = mul(output.dirLightPos, dirProj);
    
    	// Calculate the position of the vertice as viewed by the spotlight. 
    output.spotLightPosIn = mul(input.position, worldMatrix);
    output.spotLightPosIn = mul(output.spotLightPosIn, spotView);
    output.spotLightPosIn = mul(output.spotLightPosIn, spotProj);
    
    	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    
    //Calculate the normals by multiplying what is in the input by the worldMatrix and then normalize
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    //Obtain the world position
    output.worldPosition = mul(input.position, worldMatrix).xyz;
    return output;
}