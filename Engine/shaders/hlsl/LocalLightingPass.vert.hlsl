#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 20 "slang/LocalLightingPass.slang"
struct PushConstants_0
{
    float4x4 model_0;
    float4x4 viewProjection_0;
    float3 viewPosition_0;
    int lightIndex_0;
    float2 screenSize_0;
};

cbuffer pushConstants_0 : register(b1)
{
    PushConstants_0 pushConstants_0;
}

#line 15
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
};


#line 10
struct VSInput_0
{
    float3 position_1 : POSITION0;
};


#line 35
VSOutput_0 vertexMain(VSInput_0 input_0)
{
    VSOutput_0 output_0;


    output_0.position_0 = mul(pushConstants_0.viewProjection_0, mul(pushConstants_0.model_0, float4(input_0.position_1, 1.0f)));
    return output_0;
}

