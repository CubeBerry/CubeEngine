#version 450
precision mediump float;

layout(location = 0) in vec3 i_pos;

layout(location = 1) out vec2 o_uv;

layout(set = 0, binding = 0) uniform matrix
{
    mat3 m[2];
};

void main()
{
    o_uv.x = ((i_pos.x + 1) / 2);
    o_uv.y = ((i_pos.y + 1) / 2);

    vec3 p = m[0] * vec3(i_pos.x, i_pos.y, 1.0);
    gl_Position = vec4(p.x, p.y, 0.0, 1.0);
}