struct VSOutput
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

// Equirectangular Fragment Shader
static const float2 invAtan = float2(0.1591, 0.3183);

#if defined(__hlsl__)
SamplerState smp : register(s0, space1);
Texture2D equirectangularMap : register(t0, space1);
#else
[[vk::binding(0, 0)]] Sampler2D equirectangularMap;
#endif

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
float4 fragmentMain(VSOutput input) : SV_Target
{
    float2 uv = SampleSphericalMap(normalize(input.uvw));
#if defined(__hlsl__)
    float3 color = equirectangularMap.Sample(smp, uv).rgb;
#else
    float3 color = equirectangularMap.Sample(uv).rgb;
#endif

    color = ReinhardToneMapping(color);

    // Gamma Correction
    //  color = pow(color, vec3(1.0 / 2.2));

    return float4(color, 1.0);
}
