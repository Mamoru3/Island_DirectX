cbuffer MatrixBuffer : register(b0)     //Matrices passed to the buffer to use in the vertex
{
    matrix worldMatrix;         //Matrix of the world
    matrix viewMatrix;           //View matrix on the world (transforms worldspace into camera space)
    matrix projectionMatrix;    //Projection matrix on the world (transforms camera space (eye space) into screen space)
};

struct InputType
{
    float4 position : POSITION;     //Position passed in the vertex shader
    float2 tex : TEXCOORD0;         //Texture coordinates passed in the vertex shader  
};

struct OutputType
{
    float4 position : SV_POSITION;      //Position passed to the pixel shader
    float2 tex : TEXCOORD0;             //Texture coordinates passed to the pixel shader
};

OutputType main(InputType input)
{
    OutputType output;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    //Output is then passed to the pixel shader.
    return output;
}
