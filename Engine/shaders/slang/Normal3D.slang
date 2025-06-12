#define MAX_TEXTURES 500

struct VSInput
{
    float3 position;
}

struct VSOutput
{
    float4 position : SV_POSITION;
};

struct PushConstants
{
    float4x4 modelToNDC;
}
[[vk::push_constant]] PushConstants modelToNDC;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.position = mul(modelToNDC.modelToNDC, float4(input.position, 1.0));
    
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
