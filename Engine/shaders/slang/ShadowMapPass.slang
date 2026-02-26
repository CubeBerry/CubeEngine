#extension GL_EXT_nonuniform_qualifier : enable

// #pragma pack_matrix(row_major)

// Vertex Shader
struct VSInput
{
    uint position : POSITION0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float depth : TEXCOORD0;
}

struct PushConstants
{
    float4x4 decode;
    float4x4 localToNDC;
}
#if defined(__hlsl__)
ConstantBuffer<PushConstants> pushConstants : register(b0, space0);
#else
[[vk::push_constant]] PushConstants pushConstants;
#endif

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // 11, 11, 10
    float x = float(input.position & 0x7FFu);
    float y = float((input.position >> 11) & 0x7FFu);
    float z = float((input.position >> 22) & 0x3FFu);
    float3 decoded_position = mul(pushConstants.decode, float4(float3(x, y, z), 1.0)).xyz;
    output.position = mul(pushConstants.localToNDC, float4(decoded_position, 1.0));
    output.depth = output.position.z / output.position.w;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_TARGET
{
    float depth = input.depth;

    float moment1 = depth;
    float moment2 = depth * depth;
    float moment3 = moment2 * depth;
    float moment4 = moment2 * moment2;

    return float4(moment1, moment2, moment3, moment4);
}