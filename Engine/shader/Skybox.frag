#version 460

#if VULKAN
layout(location = 0) in vec3 tex_coords;
#else
in vec3 tex_coords;
#endif

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 0, binding = 0) uniform samplerCube skybox;
#else
uniform samplerCube skybox;
#endif

void main()
{
    fragmentColor = texture(skybox, tex_coords);
}