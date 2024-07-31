#version 460
precision mediump float;

#define MAX_TEXTURES 20

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;

layout(location = 0) out vec4 fragmentColor;

struct fMatrix
{
    int texIndex;
};

layout(std140, binding = 1) uniform fUniformMatrix
{
    fMatrix f_matrix[MAX_TEXTURES];
};

uniform sampler2D tex[MAX_TEXTURES];

void main()
{
    fragmentColor = vec4(1.0, 0.0, 0.0, 1.0);
}