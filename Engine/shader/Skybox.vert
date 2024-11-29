#version 460

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in vec3 i_pos;

out vec3 tex_coords;

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
layout(std140, binding = 0) uniform vUniformMatrix
#endif
{
    vMatrix matrix[MAX_MATRICES];
};

void main()
{
    tex_coords = i_pos;
    mat4 view = mat4(mat3(matrix[0].view));
    vec4 pos = matrix[0].projection * view * vec4(i_pos, 1.0);
    gl_Position = pos.xyww;
}