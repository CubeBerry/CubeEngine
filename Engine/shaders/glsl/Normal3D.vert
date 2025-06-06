#version 460 core

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec4 i_col;

layout(location = 0) out vec4 o_col;

struct vMatrix
{
    mat4 model;
    // @TODO remove after Slang's inverseModel is moved to Push Constants
    mat4 transposeInverseModel;
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
    vMatrix matrix;
};

void main()
{
    o_col = i_col;

    gl_Position = matrix.projection * matrix.view * matrix.model * vec4(i_pos, 1.0);
}