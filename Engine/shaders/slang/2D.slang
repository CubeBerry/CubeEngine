#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_TEXTURES 500

struct VSInput
{
    float2 position : POSITION0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float isTex : TEXCOORD1;
};

struct vMatrix
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4 color;
    float3 frameSize;
    float isTex;
    float3 texelPos;
    float isTexel;
}
[[vk::binding(0, 0)]] ConstantBuffer<vMatrix> matrix : register(b0);

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // output.uv.x = ((input.position.x + 1) / 2);
    // output.uv.y = ((input.position.y + 1) / 2);

    // output.uv.x = mix((input.position.x + 1) / 2, ((input.position.x + 1) / 2) * matrix[index].frameSize.x + matrix[index].texelPos.x, isTexel);
    // output.uv.y = mix((input.position.y + 1) / 2, ((input.position.y + 1) / 2) * matrix[index].frameSize.y + matrix[index].texelPos.y, isTexel);

    if (matrix.isTexel == 1.0)
    {
        output.uv.x = lerp((input.position.x + 1.0) / 2.0, ((input.position.x + 1.0) / 2.0) * matrix.frameSize.x + matrix.texelPos.x, matrix.isTexel);
        output.uv.y = lerp((input.position.y + 1.0) / 2.0, ((input.position.y + 1.0) / 2.0) * matrix.frameSize.y + matrix.texelPos.y, matrix.isTexel);
    }
    else
    {
        output.uv.x = ((input.position.x + 1.0) / 2.0);
        output.uv.y = ((input.position.y + 1.0) / 2.0);
    }

    output.isTex = matrix.isTex;
    output.color = matrix.color;

    output.position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(input.position, 0.0, 1.0))));
    
    return output;
}

struct fMatrix
{
    int texIndex;
}
[[vk::binding(0, 1)]] ConstantBuffer<fMatrix> f_matrix : register(b1);

#if defined(__hlsl__)
SamplerState smp : register(s0, space1);
Texture2D tex[MAX_TEXTURES] : register(t0, space1);
#else
[[vk::binding(1, 1)]] Sampler2D tex[MAX_TEXTURES];
#endif

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
#if defined(__hlsl__)
    float4 texColor = lerp(input.color, tex[f_matrix.texIndex].Sample(smp, input.uv), input.isTex);
#else
    float4 texColor = lerp(input.color, tex[f_matrix.texIndex].Sample(input.uv), input.isTex);
#endif

    if (texColor.a < 0.5) discard;

    // if(inIsTex == 1.0)
    // {
    //     fragmentColor = i_col * texture(tex[inTexIndex], i_uv).rgba;
    // }
    // else
    // {
    //     fragmentColor = i_col;
    // }

    return input.color * texColor;
}
