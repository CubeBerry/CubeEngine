#define PI 3.14159265358979323846f

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

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
Texture2D SSAO : register(t4, space1);
SamplerState gSampler : register(s0, space1);

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float2 texelSize = 1.0f / float2(pushConstants.screenWidth, pushConstants.screenHeight);
    float3 centerN = gNormal.SampleLevel(gSampler, input.uv, 0).xyz;
    if (length(centerN) < 0.1f) return 1.0f;

    float3 centerP = gPosition.SampleLevel(gSampler, input.uv, 0).xyz;
    float centerD = mul(pushConstants.view, float4(centerP, 1.0f)).z;

    float result = 0.0f;
    float weightSum = 0.0f;
    float variance = 0.01f;

    int w = 6;
    float s = (float)w / 2.0f;
    float s2 = 2.0f * s * s;
    for (int i = -w; i <= w; ++i)
    {
        float2 offset = float2(pushConstants.blurDirection.x * i, pushConstants.blurDirection.y * i) * texelSize;
        float2 sampleUV = input.uv + offset;

        float sampleAO = SSAO.SampleLevel(gSampler, sampleUV, 0).r;
        float3 sampleN = gNormal.SampleLevel(gSampler, sampleUV, 0).xyz;
        float3 sampleP = gPosition.SampleLevel(gSampler, sampleUV, 0).xyz;
        float sampleD = mul(pushConstants.view, float4(sampleP, 1.0f)).z;

        float spatialWeight = exp(-(i * i) / s2);

        float normalTerm = max(0.0f, dot(sampleN, centerN));
        float depthDiff = sampleD - centerD;
        float depthTerm = (1.0f / sqrt(2.0f * PI * variance)) * exp(-(depthDiff * depthDiff) / (2.0f * variance));

        float weight = spatialWeight * normalTerm * depthTerm;

        result += sampleAO * weight;
        weightSum += weight;
    }

    return result / weightSum;
}
