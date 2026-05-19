#define MAX_TEXTURES 500

struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(7)]] int4 boneIDs : BLENDINDICES0;
    [[vk::location(8)]] float4 weights : BLENDWEIGHTS0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
};

struct vMatrix
{
    float4x4 model;
    float4x4 transposeInverseModel;
    float4x4 view;
    float4x4 projection;
    float4x4 decode;
    float4 color;
    float4 viewPosition;

    // Bone matrices — same UBO as the main 3D shader (binding 0, set 0)
    float4x4 finalBones[128];
};

[[vk::binding(0, 0)]] ConstantBuffer<vMatrix> matrix : register(b0, space0);

struct PushConstants
{
    float4x4 modelToNDC;
}
#if defined(__hlsl__)
ConstantBuffer<PushConstants> modelToNDC : register(b0, space1);
#else
[[vk::push_constant]] PushConstants modelToNDC;
#endif

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // Apply skeletal skinning if this vertex is influenced by bones
    float totalWeight = input.weights[0] + input.weights[1] + input.weights[2] + input.weights[3];

    float4 skinnedPos;
    if (totalWeight > 0.01)
    {
        skinnedPos = float4(0.0, 0.0, 0.0, 0.0);
        float weightNormalizationScale = 1.0 / totalWeight;

        for (int i = 0; i < 4; i++)
        {
            if (input.boneIDs[i] < 0 || input.boneIDs[i] >= 128)
                continue;
            if (input.weights[i] <= 0.0f)
                continue;

            float normalizedWeight = input.weights[i] * weightNormalizationScale;
            skinnedPos += mul(matrix.finalBones[input.boneIDs[i]], float4(input.position, 1.0f)) * normalizedWeight;
        }
    }
    else
    {
        // Non-skinned vertex: use position directly
        skinnedPos = float4(input.position, 1.0);
    }

    // We still use the push constant modelToNDC for consistency with existing draw call
    // Or we could use matrix.projection * matrix.view * matrix.model * skinnedPos
    output.position = mul(modelToNDC.modelToNDC, skinnedPos);
    
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
