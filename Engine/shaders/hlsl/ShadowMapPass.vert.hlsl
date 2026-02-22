#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 11 "slang/ShadowMapPass.slang"
struct PushConstants_0
{
    float4x4 localToNDC_0;
};

cbuffer pushConstants_0 : register(b0)
{
    PushConstants_0 pushConstants_0;
}

#line 6
struct VSInput_0
{
    float3 position_0 : POSITION0;
};


#line 22
float4 vertexMain(VSInput_0 input_0) : SV_POSITION
{
    return mul(pushConstants_0.localToNDC_0, float4(input_0.position_0, 1.0f));
}

