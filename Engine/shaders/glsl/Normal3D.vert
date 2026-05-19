#version 460 core

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in vec3 i_pos;

// Bone skinning attributes (same locations as main 3D.vert)
layout(location = 7) in ivec4 i_boneIds;
layout(location = 8) in vec4  i_weights;

struct vMatrix
{
    mat4 model;
    // @TODO remove after Slang's inverseModel is moved to Push Constants
    mat4 transposeInverseModel;
    mat4 view;
    mat4 projection;
    mat4 decode;
    vec4 color;
    // @TODO move to push constants later
    vec3 viewPosition;

    mat4 finalBones[128];
};

#if VULKAN
layout(set = 0, binding = 0) uniform vUniformMatrix
#else
layout(std140, binding = 2) uniform vUniformMatrix
#endif
{
    vMatrix matrix;
};

void main()
{
    // Apply skeletal skinning if this vertex is influenced by bones
    float totalWeight = i_weights[0] + i_weights[1] + i_weights[2] + i_weights[3];

    vec4 skinnedPos;
    if (totalWeight > 0.01)
    {
        skinnedPos = vec4(0.0);
        float invTotal = 1.0 / totalWeight;

        for (int i = 0; i < 4; ++i)
        {
            if (i_boneIds[i] < 0 || i_boneIds[i] >= 128) continue;
            if (i_weights[i] <= 0.0) continue;

            float w = i_weights[i] * invTotal;
            skinnedPos += matrix.finalBones[i_boneIds[i]] * vec4(i_pos, 1.0) * w;
        }
    }
    else
    {
        // Non-skinned vertex: use position directly
        skinnedPos = vec4(i_pos, 1.0);
    }

    gl_Position = matrix.projection * matrix.view * matrix.model * skinnedPos;
}