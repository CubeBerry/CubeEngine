#define MAX_TEXTURES 500
#define PI 3.14159265358979323846f

struct VSInput
{
    float3 position;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 o_position;
};

// Cubemap Vertex Shader
struct WorldToNDC
{
    float4x4 view;
    float4x4 projection;
}
[[vk::push_constant]] WorldToNDC worldToNDC;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.o_position = input.position;

    output.position = mul(worldToNDC.projection, mul(worldToNDC.view, float4(input.position, 1.0)));
    
    return output;
}

// Equirectangular Fragment Shader
const float2 invAtan = float2(0.1591, 0.3183);

[[vk::binding(0, 0)]] Sampler2D equirectangularMap;

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

// Reinhard Tone Mapping
float3 ReinhardToneMapping(float3 color)
{
    return color / (color + float3(1.0));
}

// Filimic/ACES TOne Mapping
float3 FilmicToneMapping(float3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

[shader("fragment")]
float4 equirectangularFragmentMain(VSOutput input)
{
    float2 uv = SampleSphericalMap(normalize(input.o_position));
    float3 color = equirectangularMap.Sample(uv).rgb;

    color = ReinhardToneMapping(color);

    // Gamma Correction
    //  color = pow(color, vec3(1.0 / 2.2));

    return float4(color, 1.0);
}

// Irradiance Fragment Shader
[[vk::binding(0, 0)]] SamplerCube irradianceEnvironmentMap;

[shader("fragment")]
float4 irradianceFragmentMain(VSOutput input)
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

            irradiance += irradianceEnvironmentMap.Sample(sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    return float4(irradiance, 1.0);
}

// Prefiler Fragment Shader
[[vk::binding(0, 0)]] SamplerCube prefilterEnvironmentMap;

struct PrefilterUBO
{
    float roughness;
}
[[vk::binding(1, 0)]] ConstantBuffer<PrefilterUBO> roughnessUBO;

// GGX/Trowbridge-Reitz Normal Distribution Function
float DistributionGGX(float alpha, float3 N, float3 H)
{
    float numerator = pow(alpha, 2.0);

    float NdotH = max(dot(N, H), 0.0);
    float denominator = PI * pow(pow(NdotH, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float alpha)
{
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

[shader("fragment")]
float4 prefilterFragmentMain(VSOutput input)
{
    float3 N = normalize(float3(input.o_position.x, input.o_position.y, input.o_position.z));
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float3 prefilteredColor = float3(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughnessUBO.roughness * roughnessUBO.roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            float D = DistributionGGX(roughnessUBO.roughness * roughnessUBO.roughness, N, H);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float resolution = 2048.0;
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughnessUBO.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilteredColor += prefilterEnvironmentMap.SampleLevel(L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    return float4(prefilteredColor, 1.0);
}
