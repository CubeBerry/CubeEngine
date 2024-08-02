#version 460
#extension GL_EXT_nonuniform_qualifier : enable
precision mediump float;

#define MAX_MATRICES 500

layout(location = 0) in vec4 i_pos;
layout(location = 1) in vec4 i_normal;
layout(location = 2) in vec2 i_uv;
layout(location = 3) in int index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;

struct vMatrix
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
};

layout(set = 0, binding = 0) uniform vUniformMatrix
{
    vMatrix matrix[MAX_MATRICES];
};

void main()
{
    o_uv = i_uv;
    o_col = matrix[index].color;

    gl_Position = matrix[index].projection * matrix[index].view * matrix[index].model * i_pos;
}