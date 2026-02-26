#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 11 "slang/ShadowMapPass.slang"
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float depth_0 : TEXCOORD0;
};


#line 44
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 44
    float depth_1 = input_0.depth_0;

#line 49
    float moment2_0 = depth_1 * depth_1;



    return float4(input_0.depth_0, moment2_0, moment2_0 * input_0.depth_0, moment2_0 * moment2_0);
}

