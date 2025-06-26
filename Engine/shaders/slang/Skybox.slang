struct VSInput
{
    float3 position : POSITION0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

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

    float4x4 view =
        {
            float4(worldToNDC.view[0].xyz, 0.0f),
            float4(worldToNDC.view[1].xyz, 0.0f),
            float4(worldToNDC.view[2].xyz, 0.0f),
            float4(0.0f)
        };
    float4 position = mul(worldToNDC.projection, mul(view, float4(input.position, 1.0f)));
    output.position = position.xyww;
    
    return output;
}

#if defined(__hlsl__)
SamplerState smp : register(s0, space1);
TextureCube skybox : register(t0, space1);
#else
[[vk::binding(0, 0)]] SamplerCube skybox;
#endif

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
#if defined(__hlsl__)
    float3 color = skybox.Sample(smp, input.uvw).rgb;
#else
    float3 color = skybox.Sample(input.uvw).rgb;
#endif
    return float4(color, 1.0f);
}
