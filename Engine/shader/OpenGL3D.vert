#version 460

#define MAX_MATRICES 20

layout(location = 0) in vec4 i_pos;
layout(location = 1) in vec4 i_normal;
layout(location = 2) in vec2 i_uv;
layout(location = 3) in int index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;
//Lighting
layout(location = 2) out vec3 o_light_position;
layout(location = 3) out vec3 o_light_color;
layout(location = 4) out float o_ambient_strength;
layout(location = 5) out vec3 o_normal;
layout(location = 6) out vec3 o_fragment_position;
layout(location = 7) out vec3 o_view_position;
layout(location = 8) out float o_specular_strength;
layout(location = 9) out float o_is_lighting;

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

struct vLighting
{
    //Common
    vec4 lightPosition;
    vec4 lightColor;

    //Ambient
    vec4 viewPosition;
    float ambientStrength;

    //Specular
    float specularStrength;

    float isLighting;
};

layout(std140, binding = 3) uniform vLightingMatrix
{
    vLighting lightingMatrix;
};

void main()
{
    o_uv = i_uv;
    o_col = matrix[index].color;
    //Lighting
    o_light_position = lightingMatrix.lightPosition.xyz;
    o_light_color = lightingMatrix.lightColor.xyz;
    o_ambient_strength = lightingMatrix.ambientStrength;
    o_normal = (transpose(inverse(matrix[index].model)) * i_normal).xyz;
    o_fragment_position = (matrix[index].model * i_pos).xyz;
    o_view_position = lightingMatrix.viewPosition.xyz;
    o_specular_strength = lightingMatrix.specularStrength;
    o_is_lighting = lightingMatrix.isLighting;

    gl_Position = matrix[index].projection * matrix[index].view * matrix[index].model * i_pos;
}