#version 460

#if VULKAN
layout(location = 0) in vec3 i_pos;
#else
in vec3 i_pos;
#endif

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 0, binding = 0) uniform samplerCube skybox;
#else
uniform samplerCube skybox;
#endif

void main()
{
    vec3 color = texture(skybox, i_pos).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    fragmentColor = vec4(color, 1.0);
}