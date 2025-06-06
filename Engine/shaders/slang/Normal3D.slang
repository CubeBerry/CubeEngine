#define MAX_TEXTURES 500

struct VSInput
{
    float3 position;
    float4 color;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color;
};

struct vMatrix
{
    float4x4 model;
    // @TODO move to push constants
    float4x4 transposeInverseModel;
    float4x4 view;
    float4x4 projection;
    float4 color;
    // @TODO move to push constants
    float3 viewPosition;
}
[[vk::binding(0, 0)]] ConstantBuffer<vMatrix> matrix;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.color = matrix.color;

    output.position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(input.position, 1.0))));
    
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    // @TODO Fix 3D Normal Color to White (both on OpenGL and Vulkan)
    return input.color;
}
