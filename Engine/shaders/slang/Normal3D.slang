#define MAX_TEXTURES 500

struct VSInput
{
    float3 position : POSITION0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
};

struct PushConstants
{
    float4x4 modelToNDC;
}
#if defined(__hlsl__)
ConstantBuffer<PushConstants> modelToNDC : register(b0, space0);
#else
[[vk::push_constant]] PushConstants modelToNDC;
#endif

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.position = mul(modelToNDC.modelToNDC, float4(input.position, 1.0));
    
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
