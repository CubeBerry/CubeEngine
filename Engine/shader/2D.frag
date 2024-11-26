#version 460
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#if VULKAN
#define MAX_TEXTURES 500
#else
#define MAX_TEXTURES 20
#endif

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) in float inIsTex;
layout(location = 3) in flat int inIndex;

layout(location = 0) out vec4 fragmentColor;

struct fMatrix
{
    int texIndex;
};

#if VULKAN
layout(set = 1, binding = 0) uniform fUniformMatrix
#else
layout(std140, binding = 1) uniform fUniformMatrix
#endif
{
    fMatrix f_matrix[MAX_TEXTURES];
};

#if VULKAN
layout(set = 1, binding = 1) uniform sampler2D tex[MAX_TEXTURES];
#else
uniform sampler2D tex[MAX_TEXTURES];
#endif

void main()
{
    vec4 texColor = mix(i_col, texture(tex[f_matrix[inIndex].texIndex], i_uv), inIsTex);

    if (texColor.a < 0.5) discard;
    fragmentColor = i_col * texColor;

    // if(inIsTex == 1.0)
    // {
    //     fragmentColor = i_col * texture(tex[inTexIndex], i_uv).rgba;
    // }
    // else
    // {
    //     fragmentColor = i_col;
    // }
}