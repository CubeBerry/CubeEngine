#version 460 core

const vec2 invAtan = vec2(0.1591, 0.3183);

layout(location = 0) in vec3 i_pos;

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 0, binding = 0) uniform sampler2D equirectangularMap;
#else
uniform sampler2D equirectangularMap;
#endif

// @TODO Remove this function after post-process tone mapping is implemented in OpenGL
// Reinhard Tone Mapping
vec3 ReinhardToneMapping(vec3 color)
{
    return color / (color + vec3(1.0));
}

// @TODO Remove this function after post-process tone mapping is implemented in OpenGL
// Filimic/ACES Tone Mapping
vec3 FilmicToneMapping(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

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

    // @TODO Remove tone mapping after post-process tone mapping is implemented in Vulkan
    // Convert HDR to LDR
    color = FilmicToneMapping(color);
    // 2.2 Gamma Correction
    // color = pow(color, vec3(1.0 / 2.2));

    fragmentColor = vec4(color, 1.0);
}