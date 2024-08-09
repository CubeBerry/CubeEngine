#version 460
#extension GL_EXT_nonuniform_qualifier : enable
precision mediump float;

#define MAX_TEXTURES 500

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) in float inIsTex;
layout(location = 3) in flat int inIndex;

layout(location = 0) out vec4 fragmentColor;

struct fMatrix
{
    int texIndex;
};

layout(set = 1, binding = 0) uniform fUniformMatrix
{
    fMatrix f_matrix[MAX_TEXTURES];
};

layout(set = 1, binding = 1) uniform sampler2D tex[MAX_TEXTURES];

void main()
{
    if (texture(tex[f_matrix[inIndex].texIndex], i_uv).a < 0.5)
        discard;
    vec4 tColor = i_col * texture(tex[f_matrix[inIndex].texIndex], i_uv).rgba;
    fragmentColor = mix(i_col, tColor, inIsTex);

    // if(inIsTex == 1.0)
    // {
    //     fragmentColor = i_col * texture(tex[inTexIndex], i_uv).rgba;
    // }
    // else
    // {
    //     fragmentColor = i_col;
    // }
}