struct VSInput
{
    float3 position;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 o_position;
};

// Cubemap Vertex Shader
struct WorldToNDC
{
    float4x4 view;
    float4x4 projection;
}
[[vk::push_constant]] WorldToNDC worldToNDC;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.o_position = input.position;

    output.position = mul(worldToNDC.projection, mul(worldToNDC.view, float4(input.position, 1.0)));
    
    return output;
}
