#version 450
precision mediump float;

layout(location = 0) in vec3 i_col;
layout(location = 1) in vec2 i_uv;

layout(location = 0) out vec4 framgentColor;

//Uniform Block
layout(set = 0, binding = 0) uniform Material
{
    vec3 col;
} material;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main()
{
    vec3 col = i_col;
    col *= material.col;

    col *= texture(tex, i_uv).rgb;

    framgentColor = vec4(col, 1.0);
}