#extension GL_EXT_nonuniform_qualifier : enable

#define THREAD_GROUP_SIZE 128
#define MAX_WEIGHTS 101

struct BlurParams
{
    float4 weights[MAX_WEIGHTS];
    int blurWidth;
    int isVertical;
}
#if defined(__hlsl__)
ConstantBuffer<BlurParams> blurParams : register(b0, space0);
#else
[[vk::binding(0, 0)]] ConstantBuffer<BlurParams> blurParams;
#endif

#if defined(__hlsl__)
Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);
#else
Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);
#endif

groupshared float4 v[THREAD_GROUP_SIZE + MAX_WEIGHTS];

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void computeMain(
    uint3 GroupID: SV_GroupID,
    uint3 GTid: SV_GroupThreadID
)
{
    uint width, height;
    input.GetDimensions(width, height);

    int w = blurParams.blurWidth;;
    uint lpos = GTid.x;
    int2 gpos = blurParams.isVertical == 0 ? int2(GroupID.x * THREAD_GROUP_SIZE + GTid.x, GroupID.y) : int2(GroupID.x, GroupID.y * THREAD_GROUP_SIZE + GTid.x);
    int2 dir = blurParams.isVertical == 0 ? int2(1, 0) : int2(0, 1);

    int2 readPos = gpos - dir * w;
    readPos = clamp(readPos, int2(0, 0), int2(width - 1, height - 1));
    v[lpos] = input[readPos];

    if (lpos < 2 * w)
    {
        int2 extraReadPos = gpos + dir * (128 - w);
        extraReadPos = clamp(extraReadPos, int2(0, 0), int2(width - 1, height - 1));
        v[lpos + 128] = input[extraReadPos];
    }

    GroupMemoryBarrierWithGroupSync();

    float4 sum = float4(0.0);
    for (int i = 0; i <= 2 * w; i++)
    {
        sum += v[lpos + i] * blurParams.weights[i].x;
    }
    
    if (gpos.x < width && gpos.y < height)
    {
        output[gpos] = sum;
    }
}
