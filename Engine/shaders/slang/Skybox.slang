struct VSInput
{
    float3 position;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 uvw;
};

struct WorldToNDC
{
    float4x4 view;
    float4x4 projection;
}
ConstantBuffer<WorldToNDC> worldToNDC;

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

SamplerCube skybox;

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float3 color = skybox.Sample(input.uvw).rgb;
    return float4(color, 1.0f);
}