#version 460
#extension GL_EXT_nonuniform_qualifier : enable
precision mediump float;

layout(location = 2) in vec2 i_uv;
layout(location = 3) in float inTexIndex;

layout(location = 0) out vec4 framgentColor;

layout(set = 1, binding = 1) uniform sampler2D tex[];

void main()
{
    vec3 col = texture(tex[int(inTexIndex)], i_uv).rgb;
    framgentColor = vec4(col, 1.0);
}