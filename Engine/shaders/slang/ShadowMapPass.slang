#extension GL_EXT_nonuniform_qualifier : enable

// #pragma pack_matrix(row_major)

// Vertex Shader
struct VSInput
{
    uint position : POSITION0;
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
float4 vertexMain(VSInput input) : SV_POSITION
{
    // 11, 11, 10
    float x = float(input.position & 0x7FFu);
    float y = float((input.position >> 11) & 0x7FFu);
    float z = float((input.position >> 22) & 0x3FFu);
    float3 decoded_position = mul(pushConstants.decode, float4(float3(x, y, z), 1.0)).xyz;
    return mul(pushConstants.localToNDC, float4(decoded_position, 1.0));
}
