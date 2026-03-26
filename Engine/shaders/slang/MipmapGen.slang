Texture2DArray<float4> srcTexture : register(t0);
RWTexture2DArray<float4> dstTexture : register(u0);
SamplerState LinearClampSampler : register(s0);

struct MipGenConstants
{
    float2 texelSize;
};
ConstantBuffer<MipGenConstants> cb : register(b0);

[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint dstWidth, dstHeight, elements;
    dstTexture.GetDimensions(dstWidth, dstHeight, elements);

    if (dispatchThreadID.x >= dstWidth || dispatchThreadID.y >= dstHeight || dispatchThreadID.z >= elements) return;

    float2 uv = (float2(dispatchThreadID.xy) + 0.5f) * cb.texelSize;
    float4 color = srcTexture.SampleLevel(LinearClampSampler, float3(uv, dispatchThreadID.z), 0);
    dstTexture[dispatchThreadID] = color;
}
