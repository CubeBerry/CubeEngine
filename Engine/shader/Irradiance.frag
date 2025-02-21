#version 460 core

const float PI = 3.14159265359;

layout(location = 0) in vec3 i_pos;

layout(location = 0) out vec4 fragmentColor;

#if VULKAN
layout(set = 0, binding = 0) uniform samplerCube environmentMap;
#else
uniform samplerCube environmentMap;
#endif

void main()
{
    vec3 N = normalize(vec3(i_pos.x, i_pos.y, i_pos.z));
    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            //spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            //tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    fragmentColor = vec4(irradiance, 1.0);
}