// Tessellation vertex shader.
cbuffer MatrixBuffer : register(b0)         //Buffer that holds information passed from the tessellationShader.cpp
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType           //Output to go to the hull shader.
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

OutputType main(InputType input)
{
    OutputType output;

    // Pass the vertex position into the hull shader.
    output.position = input.position;

    // Pass the input color into the hull shader.
    output.normal = input.normal;
    //Pass texture data into the hulld shader
    output.tex = input.tex;
    
    return output;
}
