struct VSInput
{
    float3 position : POSITION0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

// Cubemap Vertex Shader
struct WorldToNDC
{
    float4x4 view;
    float4x4 projection;
}
#if defined(__hlsl__)
ConstantBuffer<WorldToNDC> worldToNDC : register(b0, space0);
#else
[[vk::push_constant]] WorldToNDC worldToNDC;
#endif

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.uvw = input.position;

    output.position = mul(worldToNDC.projection, mul(worldToNDC.view, float4(input.position, 1.0)));
    
    return output;
}
