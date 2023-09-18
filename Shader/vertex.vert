#version 450
precision mediump float;

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_col;
layout(location = 2) in vec2 i_uv;

layout(location = 0) out vec3 o_col;
layout(location = 1) out vec2 o_uv;

void main()
{
    gl_Position = vec4(i_pos, 1.0);
    o_col = i_col;
    o_uv = i_uv;
}