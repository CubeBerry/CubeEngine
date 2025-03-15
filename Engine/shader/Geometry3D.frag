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

struct fMaterial
{
    //Blinn-Phong
    vec3 specularColor;
    float shininess;

    //PBR
    float metallic;
    float roughness;
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
layout(set = 1, binding = 1) uniform fUniformMaterial
#else
layout(std140, binding = 4) uniform fUniformMaterial
#endif
{
    fMaterial f_material[MAX_TEXTURES];
};

#if VULKAN
layout(set = 1, binding = 2) uniform sampler2D tex[MAX_TEXTURES];
#else
//Unit 0 ~ 28
uniform sampler2D tex[MAX_TEXTURES];
#endif

// #if VULKAN
// layout(set = 1, binding = 3) uniform samplerCube irradianceMap;
// layout(set = 1, binding = 4) uniform samplerCube prefilterMap;
// layout(set = 1, binding = 5) uniform sampler2D brdfLUT;
// #else
// //Unit 29
// uniform samplerCube irradianceMap;
// //Unit 30
// uniform samplerCube prefilterMap;
// //Unit 31
// uniform sampler2D brdfLUT;
// #endif

// vec3 Froughness(vec3 F0, vec3 V, vec3 H, float roughness)
// {
//     return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);
// }

void main()
{
    vec3 albedo = vec3(0.0);
    if (f_matrix[i_object_index].isTex > 0) albedo = texture(tex[f_matrix[i_object_index].texIndex + i_tex_sub_index], i_uv).rgb;
    else albedo = i_col.rgb;

    gPosition = vec4(i_fragment_position, 1.0);
    gNormal = vec4(normalize(i_normal), 1.0);
    gAlbedo = vec4(albedo, 1.0);

    // vec3 F0 = mix(vec3(0.04), gAlbedo.rgb, f_material[i_object_index].metallic);
    // vec3 V = normalize(i_view_position - gPosition.rgb);
    // vec3 N = gNormal.rgb;

    // //PBR IBL Ambient Lighting
    // vec3 R = reflect(-V, N);
    // vec3 F = Froughness(F0, V, N, f_material[i_object_index].roughness);

    // vec3 Ks = F;
    // vec3 Kd = (1.0 - f_material[i_object_index].metallic) * (vec3(1.0) - Ks);
    // vec3 irradiance = texture(irradianceMap, N).rgb;
    // vec3 diffuse = irradiance * gAlbedo.rgb;

    // const float MAX_REFLECTION_LOD = 4.0;
    // vec3 prefilteredColor = textureLod(prefilterMap, R, f_material[i_object_index].roughness * MAX_REFLECTION_LOD).rgb;
    // vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), f_material[i_object_index].roughness)).rg;
    // vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    // //1.0 == ao
    // vec3 ambient = (Kd * diffuse + specular) * 1.0;
    // vec3 resultColor = gAlbedo.rgb;
    // resultColor = ambient.rgb + resultColor;
    // gAlbedo = vec4(resultColor, 1.0);
}