#version 460

in vec3 tex_coords;

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 1, binding = 0) uniform samplerCube skybox;
#else
uniform samplerCube skybox;
#endif

void main()
{
    fragmentColor = texture(skybox, tex_coords);
}