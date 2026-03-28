#define PI 3.14159265358979323846f

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

// Irradiance Fragment Shader
// #if defined(__hlsl__)
// SamplerState smp : register(s0, space1);
// TextureCube environmentMap : register(t0, space1);
// #else
// [[vk::binding(0, 0)]] SamplerCube environmentMap;
// #endif

struct SHCoefficients
{
    float4 E_lm[9];
};
ConstantBuffer<SHCoefficients> shBuffer : register(b1, space0);

// [shader("fragment")]
// float4 fragmentMain(VSOutput input) : SV_Target
// {
//     float3 N = normalize(float3(input.uvw.x, input.uvw.y, input.uvw.z));
//     float3 irradiance = float3(0.0);

//     float3 up = float3(0.0, 1.0, 0.0);
//     float3 right = normalize(cross(up, N));
//     up = normalize(cross(N, right));

//     float sampleDelta = 0.025;
//     float nrSamples = 0.0;
//     for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
//     {
//         for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
//         {
//             // spherical to cartesian (in tangent space)
//             float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
//             // tangent space to world
//             float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

// #if defined(__hlsl__)
//             irradiance += environmentMap.Sample(smp, sampleVec).rgb * cos(theta) * sin(theta);
// #else
//             irradiance += environmentMap.Sample(sampleVec).rgb * cos(theta) * sin(theta);
// #endif
//             nrSamples++;
//         }
//     }
//     irradiance = PI * irradiance * (1.0 / float(nrSamples));

//     return float4(irradiance, 1.0);
// }

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 N = normalize(float3(input.uvw.x, input.uvw.y, input.uvw.z));
    float x = N.x;
    float y = N.y;
    float z = N.z;

    float Y00 = 0.5 * sqrt(1.0 / PI);
    float Y1_1 = (0.5 * sqrt(3.0 / PI)) * y;
    float Y10 = (0.5 * sqrt(3.0 / PI)) * z;
    float Y11 = (0.5 * sqrt(3.0 / PI)) * x;
    float Y2_2 = (0.5 * sqrt(15.0 / PI)) * x * y;
    float Y2_1 = (0.5 * sqrt(15.0 / PI)) * y * z;
    float Y20 = (0.25 * sqrt(5.0 / PI)) * (3.0 * z * z - 1.0);
    float Y21 = (0.5 * sqrt(15.0 / PI)) * x * z;
    float Y22 = (0.25 * sqrt(15.0 / PI)) * (x * x - y * y);

    float3 irradiance =
        shBuffer.E_lm[0].rgb * Y00 +
        shBuffer.E_lm[1].rgb * Y1_1 +
        shBuffer.E_lm[2].rgb * Y10 +
        shBuffer.E_lm[3].rgb * Y11 +
        shBuffer.E_lm[4].rgb * Y2_2 +
        shBuffer.E_lm[5].rgb * Y2_1 +
        shBuffer.E_lm[6].rgb * Y20 +
        shBuffer.E_lm[7].rgb * Y21 +
        shBuffer.E_lm[8].rgb * Y22;

    return float4(irradiance, 1.0);
}