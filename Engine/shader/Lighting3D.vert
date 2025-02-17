#version 460 core
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#if VULKAN
#define MAX_MATRICES 500
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in vec2 i_pos;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in int object_index;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out int o_object_index;

void main()
{
    o_uv = i_uv;
    o_object_index = object_index;
    gl_Position = vec4(i_pos, 0.0, 1.0);
}