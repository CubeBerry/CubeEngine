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

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            float2 sampleUV = input.uv + offset;

            float sampleAO = SSAO.SampleLevel(gSampler, sampleUV, 0).r;
            float3 sampleN = gNormal.SampleLevel(gSampler, sampleUV, 0).xyz;
            float3 sampleP = gPosition.SampleLevel(gSampler, sampleUV, 0).xyz;
            float sampleD = mul(pushConstants.view, float4(sampleP, 1.0f)).z;

            float spatialWeight = exp(-(x * x + y * y) / 8.0f);

            float normalTerm = max(0.0f, dot(sampleN, centerN));
            float depthDiff = sampleD - centerD;
            float depthTerm = (1.0f / sqrt(2.0f * 3.141592f * variance)) * exp(-(depthDiff * depthDiff) / (2.0f * variance));

            float rangeWeight = normalTerm * depthTerm;

            float weight = spatialWeight * rangeWeight;

            result += sampleAO * weight;
            weightSum += weight;
        }
    }

    return result / max(weightSum, 0.0001f);
}
