// Tessellation Hull Shader
// Prepares control points for tessellation
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

struct ConstantOutputType       //Cube, has 2 inside edges and four outside edges
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType       //Output to go to the domain shader
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

cbuffer EdgesValues : register(b1)      //Buffer in register 1 passed from the tessellationShader
{
    float3 camera_pos;
    float tess_res;
};
                    
ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    //---------------------------------Distance calculated from the camera to each side of the square to avoid popping (divided by 0.5 to minimize popping)
    float3 dist1 = camera_pos - ((inputPatch[1].position + inputPatch[0].position) * 0.5);
    float3 dist2 = camera_pos - ((inputPatch[2].position + inputPatch[1].position) * 0.5);
    float3 dist3 = camera_pos - ((inputPatch[3].position + inputPatch[2].position) * 0.5);
    float3 dist4 = camera_pos - ((inputPatch[0].position + inputPatch[0].position) * 0.5);
    
        //---------------------------------Distance calculated from the camera to the inside edges of the square
    float3 dist5 = camera_pos - ((inputPatch[0].position + inputPatch[2].position) * 0.5);
    float3 dist6 = camera_pos - ((inputPatch[1].position + inputPatch[3].position) * 0.5);
    
    float3 lengthDist1 = length(dist1);         //Extract the lenght from the distance vector
    float3 lengthDist2 = length(dist2);
    float3 lengthDist3 = length(dist3);
    float3 lengthDist4 = length(dist4);
    float3 lengthDist5 = length(dist5);
    float3 lengthDist6 = length(dist6);
    
    float tessellationFact1 =(1 / lengthDist1) * tess_res;   //Tessellation factor for each of the sides, the tessellation  is first divided by one not to make the tessellation too high and
    float tessellationFact2 =(1 / lengthDist2) * tess_res;   //Is multiplied by the tessellation resolution passed from the app1.cpp, making the terrain dynamically tessellated
    float tessellationFact3 =(1 / lengthDist3) * tess_res;
    float tessellationFact4 =(1 / lengthDist4) * tess_res;
    float tessellationFact5 =(1 / lengthDist5) * tess_res;
    float tessellationFact6 =(1 / lengthDist6) * tess_res;
    
    output.edges[0] = tessellationFact1;            //the order of the edges is inverted for each tessellation factor, instead of 0123, 0321(clockwise) by trying the orders, this one is the
    output.edges[1] = tessellationFact2;            //One that makes the popping less noticeable
    output.edges[2] = tessellationFact3;
    output.edges[3] = tessellationFact4;
    output.inside[0] = tessellationFact5;
    output.inside[1] = tessellationFact6;
    
    return output;
}


[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;
    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;
    // Set the input colour as the output colour.
    output.normal = patch[pointId].normal;
    //Set the input world position as the output world position
    output.worldPosition = patch[pointId].worldPosition;
    //set the output texture coordinates as the input ones
    output.tex = patch[pointId].tex;
    
    return output;
}
