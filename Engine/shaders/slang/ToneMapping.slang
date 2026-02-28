struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertexMain(uint id : SV_VertexID)
{
    VSOutput output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}

// Reinhard Tone Mapping
float3 ReinhardToneMapping(float3 color)
{
    return color / (color + float3(1.0));
}

// Filimic/ACES Tone Mapping
float3 FilmicToneMapping(float3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

SamplerState gSampler : register(s0, space1);
Texture2D gTexture : register(t0, space0);

struct PushConstants
{
    float exposure;
};
ConstantBuffer<PushConstants> pushConstants : register(b0, space0);

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float4 hdrColor = gTexture.Sample(gSampler, input.uv);
    float3 color = hdrColor.rgb;

    // Apply exposure
    color *= pushConstants.exposure;

    // Convert HDR to LDR
    color = FilmicToneMapping(color);
    // 2.2 Gamma Correction
    color = pow(color, 1.0 / 2.2);
    return float4(color, 1.0f);
}
