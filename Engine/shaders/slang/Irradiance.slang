#define PI 3.14159265358979323846f

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 o_position : TEXCOORD0;
};

// Irradiance Fragment Shader
#if defined(__hlsl__)
SamplerState smp : register(s0, space0);
TextureCube environmentMap : register(t0, space0);
#else
[[vk::binding(0, 0)]] SamplerCube environmentMap;
#endif

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float3 N = normalize(float3(input.o_position.x, input.o_position.y, input.o_position.z));
    float3 irradiance = float3(0.0);

    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

#if defined(__hlsl__)
            irradiance += environmentMap.Sample(smp, sampleVec).rgb * cos(theta) * sin(theta);
#else
            irradiance += environmentMap.Sample(sampleVec).rgb * cos(theta) * sin(theta);
#endif
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    return float4(irradiance, 1.0);
}
