#version 460

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_tex;

layout(location = 0) out vec2 o_tex;

void main()
{
    o_tex = i_tex;

    gl_Position = vec4(i_pos, 1.0);
}