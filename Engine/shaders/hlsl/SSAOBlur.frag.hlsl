#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 9 "slang/SSAOBlur.slang"
struct PushConstants_0
{
    float4x4 view_0;
    float4x4 projection_0;
    int2 blurDirection_0;
    float radius_0;
    float scale_0;
    float contrast_0;
    int numSamples_0;
    float delta_0;
    float screenWidth_0;
    float screenHeight_0;
};

cbuffer pushConstants_0 : register(b0)
{
    PushConstants_0 pushConstants_0;
}


Texture2D<float4 > gNormal_0 : register(t1, space1);



SamplerState gSampler_0 : register(s0, space1);


#line 30
Texture2D<float4 > gPosition_0 : register(t2, space1);

Texture2D<float4 > SSAO_0 : register(t4, space1);


#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 36
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 36
    VSOutput_0 _S1 = input_0;

    float2 _S2 = 1.0f / float2(pushConstants_0.screenWidth_0, pushConstants_0.screenHeight_0);
    float3 centerN_0 = gNormal_0.SampleLevel(gSampler_0, input_0.uv_0, 0.0f).xyz;
    if((length(centerN_0)) < 0.10000000149011612f)
    {

#line 40
        return (float4)1.0f;
    }

    float _S3 = mul(pushConstants_0.view_0, float4(gPosition_0.SampleLevel(gSampler_0, _S1.uv_0, 0.0f).xyz, 1.0f)).z;

#line 43
    int i_0 = int(-6);

#line 43
    float result_0 = 0.0f;

#line 43
    float weightSum_0 = 0.0f;

#line 52
    for(;;)
    {

#line 52
        if(i_0 <= int(6))
        {
        }
        else
        {

#line 52
            break;
        }

        float2 sampleUV_0 = _S1.uv_0 + float2(float(pushConstants_0.blurDirection_0.x * i_0), float(pushConstants_0.blurDirection_0.y * i_0)) * _S2;

#line 65
        float depthDiff_0 = mul(pushConstants_0.view_0, float4(gPosition_0.SampleLevel(gSampler_0, sampleUV_0, 0.0f).xyz, 1.0f)).z - _S3;


        float weight_0 = exp(float(- (i_0 * i_0)) / 18.0f) * max(0.0f, dot(gNormal_0.SampleLevel(gSampler_0, sampleUV_0, 0.0f).xyz, centerN_0)) * (1.0f / sqrt(0.06283185631036758f) * exp(- (depthDiff_0 * depthDiff_0) / 0.01999999955296516f));

        float result_1 = result_0 + SSAO_0.SampleLevel(gSampler_0, sampleUV_0, 0.0f).x * weight_0;
        float weightSum_1 = weightSum_0 + weight_0;

#line 52
        i_0 = i_0 + int(1);

#line 52
        result_0 = result_1;

#line 52
        weightSum_0 = weightSum_1;

#line 52
    }

#line 74
    return (float4)(result_0 / weightSum_0);
}

