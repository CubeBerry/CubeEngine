#version 450
precision mediump float;

layout(location = 1) in vec2 i_uv;

layout(location = 0) out vec4 framgentColor;

layout(set = 0, binding = 0) uniform Material
{
  mat3 matrix;  
};
layout(set = 1, binding = 1) uniform sampler2D tex;

void main()
{
    vec3 col = texture(tex, i_uv).rgb;

    framgentColor = vec4(col, 1.0);
}