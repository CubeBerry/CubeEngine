#version 460 core

layout(location = 0) out vec2 o_tex;

void main()
{
    const vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),   // Bottom-Left
        vec2(3.0, -1.0),    // Right
        vec2(-1.0, 3.0)     // Top-Left
        );
    vec2 position = positions[gl_VertexID];

    o_tex = position * 0.5 + 0.5;

    gl_Position = vec4(position, 0.0, 1.0);
}