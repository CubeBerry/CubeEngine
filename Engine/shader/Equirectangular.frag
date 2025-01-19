#version 460

const vec2 invAtan = vec2(0.1591, 0.3183);

#if VULKAN
layout(location = 0) in vec3 i_pos;
#else
in vec3 i_pos;
#endif

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 0, binding = 0) uniform sampler2D equirectangularMap;
#else
uniform sampler2D equirectangularMap;
#endif

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(i_pos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    fragmentColor = vec4(color, 1.0);
}