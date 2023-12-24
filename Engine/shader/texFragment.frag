#version 460
#extension GL_EXT_nonuniform_qualifier : enable
precision mediump float;

#define MAX_TEXTURES 500

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) flat in int inTexIndex;
layout(location = 3) in float inIsTex;

layout(location = 0) out vec4 fragmentColor;

layout(set = 1, binding = 1) uniform sampler2D tex[MAX_TEXTURES];

void main()
{
    fragmentColor = vec4(mix(i_col.xyz, texture(tex[inTexIndex], i_uv).rgb, inIsTex), 1.0);
}