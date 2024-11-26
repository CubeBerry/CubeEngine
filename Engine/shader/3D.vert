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
layout(location = 3) in int index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;
layout(location = 2) out int o_index;
//Lighting
layout(location = 3) out vec3 o_light_position;
layout(location = 4) out vec3 o_light_color;
layout(location = 5) out float o_ambient_strength;
layout(location = 6) out vec3 o_normal;
layout(location = 7) out vec3 o_fragment_position;
layout(location = 8) out vec3 o_view_position;
layout(location = 9) out float o_specular_strength;
layout(location = 10) out float o_is_lighting;

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

struct vLighting
{
    vec3 lightPosition;
    float ambientStrength;
    vec3 lightColor;
    float specularStrength;
    vec3 viewPosition;
    float isLighting;
};

#if VULKAN
layout(set = 0, binding = 1) uniform vLightingMatrix
#else
layout(std140, binding = 3) uniform vLightingMatrix
#endif
{
    vLighting lightingMatrix;
};

void main()
{
    o_uv = i_uv;
    o_col = matrix[index].color;
    o_index = index;
    //Lighting
    o_light_position = lightingMatrix.lightPosition;
    o_light_color = lightingMatrix.lightColor;
    o_ambient_strength = lightingMatrix.ambientStrength;
    o_normal = mat3(transpose(inverse(matrix[index].model))) * i_normal;
    o_fragment_position = mat3(matrix[index].model) * i_pos;
    o_view_position = lightingMatrix.viewPosition;
    o_specular_strength = lightingMatrix.specularStrength;
    o_is_lighting = lightingMatrix.isLighting;

    gl_Position = matrix[index].projection * matrix[index].view * matrix[index].model * vec4(i_pos, 1.0);
}