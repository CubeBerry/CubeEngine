#version 460
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_uv;
layout(location = 3) in int object_index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;
layout(location = 2) out int o_object_index;
//Lighting
layout(location = 3) out vec3 o_normal;
layout(location = 4) out vec3 o_fragment_position;
layout(location = 5) out vec3 o_view_position;

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
    o_uv = i_uv;
    o_col = matrix[object_index].color;
    o_object_index = object_index;
    //Lighting
    // o_normal = vec3(transpose(inverse(matrix[object_index].model)) * vec4(i_normal, 0.0));
    o_normal = mat3(matrix[object_index].model) * i_normal;
    o_fragment_position = vec3(matrix[object_index].model * vec4(i_pos, 1.0));
    o_view_position = inverse(matrix[object_index].projection * matrix[object_index].view)[3].xyz;

    gl_Position = matrix[object_index].projection * matrix[object_index].view * matrix[object_index].model * vec4(i_pos, 1.0);
}