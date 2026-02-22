#extension GL_EXT_nonuniform_qualifier : enable

// #pragma pack_matrix(row_major)

// Vertex Shader
struct VSInput
{
    float3 position : POSITION0;
}

struct PushConstants
{
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
    return mul(pushConstants.localToNDC, float4(input.position, 1.0));
}
