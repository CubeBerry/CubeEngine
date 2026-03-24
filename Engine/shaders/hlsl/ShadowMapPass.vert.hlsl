#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 17 "slang/ShadowMapPass.slang"
struct PushConstants_0
{
    float4x4 decode_0;
    float4x4 localToNDC_0;
};

cbuffer pushConstants_0 : register(b0)
{
    PushConstants_0 pushConstants_0;
}

#line 11
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float depth_0 : TEXCOORD0;
};


#line 6
struct VSInput_0
{
    uint position_1 : POSITION0;
};


#line 29
VSOutput_0 vertexMain(VSInput_0 input_0)
{
    VSOutput_0 output_0;

#line 38
    float4 _S1 = mul(pushConstants_0.localToNDC_0, float4(mul(pushConstants_0.decode_0, float4(float3(float((input_0.position_1) & 2047U), float(((input_0.position_1) >> int(11)) & 2047U), float(((input_0.position_1) >> int(22)) & 1023U)), 1.0f)).xyz, 1.0f));

#line 38
    output_0.position_0 = _S1;
    output_0.depth_0 = _S1.z / _S1.w;
    return output_0;
}

