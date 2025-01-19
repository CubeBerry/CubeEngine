#version 460

layout(location = 0) in vec3 i_pos;

#if VULKAN
layout(location = 0) out vec3 o_pos;
#else
out vec3 o_pos;
#endif

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
    mat4 view = mat4(mat3(worldToNDC.view));
    vec4 pos = worldToNDC.projection * view * vec4(i_pos, 1.0);
#else
    mat4 view = mat4(mat3(view));
    vec4 pos = projection * view * vec4(i_pos, 1.0);
#endif

    gl_Position = pos.xyww;
}