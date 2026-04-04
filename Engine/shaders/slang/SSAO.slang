#define PI 3.14159265358979323846f

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
    output.position = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    // Invert Y for DirectX coordinate system
#if defined(__hlsl__)
    output.position.y = -output.position.y;
#endif
    return output;
}

struct PushConstants
{
    float4x4 view;
    float4x4 projection;
    int2 blurDirection;
    float radius;
    float scale;
    float contrast;
    int numSamples;
    float delta;
    float screenWidth;
    float screenHeight;
}
#if defined(__hlsl__)
ConstantBuffer<PushConstants> pushConstants : register(b0, space0);
#else
[[vk::push_constant]] PushConstants pushConstants;
#endif

Texture2D gAlbedo : register(t0, space1);
Texture2D gNormal : register(t1, space1);
Texture2D gPosition : register(t2, space1);
Texture2D gMaterial : register(t3, space1);
SamplerState gSampler : register(s0, space1);

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 P = gPosition.SampleLevel(gSampler, input.uv, 0).xyz;
    float3 N = gNormal.SampleLevel(gSampler, input.uv, 0).xyz;
    // Ignore skybox pixels with near-zero normals
    if (length(N) < 0.1f) return 1.0f;

    float4 viewPosition = mul(pushConstants.view, float4(P, 1.0f));
    float d = viewPosition.z;

    int2 coord = int2(input.position.xy);
    float phi = (float)(((30 * coord.x) ^ coord.y) + 10 * coord.x * coord.y);

    float S = 0.0f;
    float c = 0.1f * pushConstants.radius;

    for (int i = 0; i < pushConstants.numSamples; ++i)
    {
        float alpha = ((float)i + 0.5f) / (float)pushConstants.numSamples;
        float h = alpha * pushConstants.radius / d;
        float theta = 2.f * PI * alpha * (7.0f * (float)pushConstants.numSamples / 9.0f) + phi;

        float2 offset = h * float2(cos(theta), sin(theta));
        // float2 sampleUV = input.uv + offset * float2(pushConstants.projection[1][1] / pushConstants.projection[0][0], 1.0f) * 0.5f;
        float2 sampleUV = input.uv + offset;

        float3 Pi = gPosition.SampleLevel(gSampler, sampleUV, 0).xyz;
        float3 wi = Pi - P;
        float di = mul(pushConstants.view, float4(Pi, 1.0f)).z;
        float heaviside = step(length(wi), pushConstants.radius);

        float numerator = max(0.0f, dot(N, wi) - pushConstants.delta * di) * heaviside;
        float denominator = max(c * c, dot(wi, wi));

        S += numerator / denominator;
    }

    S *= (2.0f * PI * c) / (float)pushConstants.numSamples;
    float A = pow(max(0.0f, 1.0f - pushConstants.scale * S), pushConstants.contrast);
    return A;
}
