#version 460 core
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#if VULKAN
#define MAX_TEXTURES 500
#else
#define MAX_TEXTURES 29
#endif

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) in flat int i_object_index;
layout(location = 3) in flat int i_tex_sub_index;
//Lighting
layout(location = 4) in vec3 i_normal;
layout(location = 5) in vec3 i_fragment_position;
layout(location = 6) in vec3 i_view_position;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;

struct fMatrix
{
    int isTex;
    int texIndex;
};

#if VULKAN
layout(set = 1, binding = 0) uniform fUniformMatrix
#else
layout(std140, binding = 3) uniform fUniformMatrix
#endif
{
    fMatrix f_matrix[MAX_TEXTURES];
};

#if VULKAN
layout(set = 1, binding = 1) uniform sampler2D tex[MAX_TEXTURES];
#else
//Unit 0 ~ 28
uniform sampler2D tex[MAX_TEXTURES];
#endif

void main()
{
    vec3 albedo = vec3(0.0);
    if (f_matrix[i_object_index].isTex > 0) albedo = texture(tex[f_matrix[i_object_index].texIndex + i_tex_sub_index], i_uv).rgb;
    else albedo = i_col.rgb;

    gPosition = vec4(i_fragment_position, 1.0);
    gNormal = vec4(normalize(i_normal), 1.0);
    gAlbedo = vec4(albedo, 1.0);
}