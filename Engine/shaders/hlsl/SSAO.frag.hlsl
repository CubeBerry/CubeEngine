#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 43 "slang/SSAO.slang"
Texture2D<float4 > gPosition_0 : register(t2, space1);

SamplerState gSampler_0 : register(s0, space1);


#line 42
Texture2D<float4 > gNormal_0 : register(t1, space1);


#line 22
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

#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 48
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 48
    VSOutput_0 _S1 = input_0;

    float3 P_0 = gPosition_0.SampleLevel(gSampler_0, input_0.uv_0, 0.0f).xyz;
    float3 N_0 = gNormal_0.SampleLevel(gSampler_0, input_0.uv_0, 0.0f).xyz;

    if((length(N_0)) < 0.10000000149011612f)
    {

#line 53
        return (float4)1.0f;
    }

    float _S2 = mul(pushConstants_0.view_0, float4(P_0, 1.0f)).z;

    int2 _S3 = int2(_S1.position_0.xy);
    int _S4 = _S3.x;

#line 59
    int _S5 = _S3.y;

#line 59
    float _S6 = float(((int(30) * _S4) ^ _S5) + int(10) * _S4 * _S5);


    float c_0 = 0.10000000149011612f * pushConstants_0.radius_0;

#line 62
    int i_0 = int(0);

#line 62
    float S_0 = 0.0f;

    for(;;)
    {

#line 64
        if(i_0 < (pushConstants_0.numSamples_0))
        {
        }
        else
        {

#line 64
            break;
        }
        float alpha_0 = (float(i_0) + 0.5f) / float(pushConstants_0.numSamples_0);

        float theta_0 = 6.28318548202514648f * alpha_0 * (7.0f * float(pushConstants_0.numSamples_0) / 9.0f) + _S6;

#line 74
        float3 Pi_0 = gPosition_0.SampleLevel(gSampler_0, _S1.uv_0 + alpha_0 * pushConstants_0.radius_0 / _S2 * float2(cos(theta_0), sin(theta_0)), 0.0f).xyz;
        float3 wi_0 = Pi_0 - P_0;

#line 82
        float S_1 = S_0 + max(0.0f, dot(N_0, wi_0) - pushConstants_0.delta_0 * mul(pushConstants_0.view_0, float4(Pi_0, 1.0f)).z) * step(length(wi_0), pushConstants_0.radius_0) / max(c_0 * c_0, dot(wi_0, wi_0));

#line 64
        i_0 = i_0 + int(1);

#line 64
        S_0 = S_1;

#line 64
    }

#line 87
    return (float4)pow(max(0.0f, 1.0f - pushConstants_0.scale_0 * (S_0 * (6.28318548202514648f * c_0 / float(pushConstants_0.numSamples_0)))), pushConstants_0.contrast_0);
}

