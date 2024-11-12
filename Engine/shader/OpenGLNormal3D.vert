#version 460

#define MAX_MATRICES 20

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec4 i_col;
layout(location = 2) in int index;

layout(location = 0) out vec4 o_col;

struct vMatrix
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
};

layout(std140, binding = 0) uniform vUniformMatrix
{
    vMatrix matrix[MAX_MATRICES];
};

void main()
{
    o_col = i_col;

    gl_Position = matrix[index].projection * matrix[index].view * matrix[index].model * vec4(i_pos, 1.0);
}