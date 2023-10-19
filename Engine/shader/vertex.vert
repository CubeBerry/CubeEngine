#version 450
precision mediump float;

layout(location = 0) in vec3 i_pos;

layout(location = 1) out vec2 o_uv;

layout(set = 0, binding = 0) uniform matrix
{
    mat3 m;
};

void main()
{
    o_uv.x = ((i_pos.x + 1) / 2);
    o_uv.y = ((i_pos.y + 1) / 2);

    gl_Position = vec4(i_pos, 1.0);
}