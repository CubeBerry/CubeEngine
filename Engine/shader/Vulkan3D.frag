#version 460
#extension GL_EXT_nonuniform_qualifier : enable
precision mediump float;

#define MAX_TEXTURES 500

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;

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
    fragmentColor = i_col;
}