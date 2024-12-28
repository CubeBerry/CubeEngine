#version 460

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

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

#if VULKAN
layout(set = 0, binding = 0) uniform vUniformMatrix
#else
layout(std140, binding = 2) uniform vUniformMatrix
#endif
{
    vMatrix matrix[MAX_MATRICES];
};

void main()
{
    o_col = i_col;

    gl_Position = matrix[index].projection * matrix[index].view * matrix[index].model * vec4(i_pos, 1.0);
}