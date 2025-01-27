#version 460 core

layout(location = 0) in vec3 i_pos;

layout(location = 0) out vec3 o_pos;

#if VULKAN
layout(push_constant) uniform WorldToNDC
{
    mat4 view;
    mat4 projection;
} worldToNDC;
#else
uniform mat4 view;
uniform mat4 projection;
#endif

void main()
{
    o_pos = i_pos;

#if VULKAN
    gl_Position = worldToNDC.projection * worldToNDC.view * vec4(i_pos, 1.0);
#else
    gl_Position = projection * view * vec4(i_pos, 1.0);
#endif
}