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


#line 29
Texture2D<float4 > gPosition_0 : register(t2, space1);

Texture2D<float4 > SSAO_0 : register(t4, space1);


#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 35
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 35
    VSOutput_0 _S1 = input_0;

    float2 _S2 = 1.0f / float2(pushConstants_0.screenWidth_0, pushConstants_0.screenHeight_0);
    float3 centerN_0 = gNormal_0.SampleLevel(gSampler_0, input_0.uv_0, 0.0f).xyz;
    if((length(centerN_0)) < 0.10000000149011612f)
    {

#line 39
        return (float4)1.0f;
    }

    float _S3 = mul(pushConstants_0.view_0, float4(gPosition_0.SampleLevel(gSampler_0, _S1.uv_0, 0.0f).xyz, 1.0f)).z;

#line 42
    int x_0 = int(-2);

#line 42
    float result_0 = 0.0f;

#line 42
    float weightSum_0 = 0.0f;

#line 48
    for(;;)
    {

#line 48
        if(x_0 <= int(2))
        {
        }
        else
        {

#line 48
            break;
        }

#line 48
        int y_0 = int(-2);

        for(;;)
        {

#line 50
            if(y_0 <= int(2))
            {
            }
            else
            {

#line 50
                break;
            }

            float2 sampleUV_0 = _S1.uv_0 + float2(float(x_0), float(y_0)) * _S2;

#line 63
            float depthDiff_0 = mul(pushConstants_0.view_0, float4(gPosition_0.SampleLevel(gSampler_0, sampleUV_0, 0.0f).xyz, 1.0f)).z - _S3;

#line 68
            float weight_0 = exp(float(- (x_0 * x_0 + y_0 * y_0)) / 8.0f) * (max(0.0f, dot(gNormal_0.SampleLevel(gSampler_0, sampleUV_0, 0.0f).xyz, centerN_0)) * (1.0f / sqrt(0.06283184140920639f) * exp(- (depthDiff_0 * depthDiff_0) / 0.01999999955296516f)));

            float result_1 = result_0 + SSAO_0.SampleLevel(gSampler_0, sampleUV_0, 0.0f).x * weight_0;
            float weightSum_1 = weightSum_0 + weight_0;

#line 50
            y_0 = y_0 + int(1);

#line 50
            result_0 = result_1;

#line 50
            weightSum_0 = weightSum_1;

#line 50
        }

#line 48
        x_0 = x_0 + int(1);

#line 48
    }

#line 75
    return (float4)(result_0 / max(weightSum_0, 0.00009999999747379f));
}

