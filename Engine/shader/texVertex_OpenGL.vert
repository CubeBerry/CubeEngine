#version 460
#extension GL_KHR_vulkan_glsl : enable //layout Set
precision mediump float;

#define MAX_MATRICES 20

layout(location = 0) in vec4 i_pos;
layout(location = 1) in int index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;
layout(location = 2) out float outIsTex;
layout(location = 3) out int outIndex;

struct vMatrix
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    vec4 frameSize;
    vec4 texelPos;
    float isTex;
    float isTexel;
};

layout(set = 0, binding = 0) uniform vUniformMatrix
{
    vMatrix matrix[MAX_MATRICES];
};

void main()
{
    //o_uv.x = ((i_pos.x + 1) / 2);
    //o_uv.y = ((i_pos.y + 1) / 2);

    //o_uv.x = mix((i_pos.x + 1) / 2, ((i_pos.x + 1) / 2) * matrix[index].frameSize.x + matrix[index].texelPos.x, isTexel);
    //o_uv.y = mix((i_pos.y + 1) / 2, ((i_pos.y + 1) / 2) * matrix[index].frameSize.y + matrix[index].texelPos.y, isTexel);

    if(matrix[index].isTexel == 1.0)
    {
        o_uv.x = mix((i_pos.x + 1.0) / 2.0, ((i_pos.x + 1.0) / 2.0) * matrix[index].frameSize.x + matrix[index].texelPos.x, matrix[index].isTexel);
        o_uv.y = mix((i_pos.y + 1.0) / 2.0, ((i_pos.y + 1.0) / 2.0) * matrix[index].frameSize.y + matrix[index].texelPos.y, matrix[index].isTexel);
    }
    else
    {
        o_uv.x = ((i_pos.x + 1.0) / 2.0);
        o_uv.y = ((i_pos.y + 1.0) / 2.0);
    }

    outIsTex = matrix[index].isTex;
    o_col = matrix[index].color;
    outIndex = index;

    //gl_Position = matrix[index].projection * matrix[index].view * matrix[index].model * i_pos;
    gl_Position = matrix[index].model * i_pos;
}