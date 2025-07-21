#version 460 core
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in uint i_pos;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_uv;
layout(location = 3) in int tex_sub_index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;
layout(location = 2) out int o_tex_sub_index;
//Lighting
layout(location = 4) out vec3 o_normal;
layout(location = 5) out vec3 o_fragment_position;
layout(location = 6) out vec3 o_view_position;

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
    // 11, 11, 10
    float x = float(i_pos & 0x7FFu);
    float y = float((i_pos >> 11) & 0x7FFu);
    float z = float((i_pos >> 22) & 0x3FFu);
    vec3 decoded_position = (matrix.decode * vec4(vec3(x, y, z), 1.0)).xyz;

    o_uv = i_uv;
    o_col = matrix.color;
    o_tex_sub_index = tex_sub_index;
    //Lighting
    // o_normal = vec3(transpose(inverse(matrix[object_index].model)) * vec4(i_normal, 0.0));
    o_normal = mat3(transpose(inverse(matrix.model))) * i_normal;
    //o_normal = mat3(matrix[object_index].model) * i_normal;
    o_fragment_position = vec3(matrix.model * vec4(decoded_position, 1.0));
    o_view_position = inverse(matrix.view)[3].xyz;

    gl_Position = matrix.projection * matrix.view * matrix.model * vec4(decoded_position, 1.0);
}