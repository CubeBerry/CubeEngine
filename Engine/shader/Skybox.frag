#version 460 core

layout(location = 0) in vec3 i_pos;

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 0, binding = 0) uniform samplerCube skybox;
#else
uniform samplerCube skybox;
#endif

void main()
{
    vec3 color = texture(skybox, i_pos).rgb;

    fragmentColor = vec4(color, 1.0);
}