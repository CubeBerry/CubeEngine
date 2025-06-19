#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 9 "BRDF.slang"
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 3
struct VSInput_0
{
    float3 position_1 : POSITION0;
    float2 uv_1 : TEXCOORD0;
};


#line 16
VSOutput_0 vertexMain(VSInput_0 input_0)
{
    VSOutput_0 output_0;

    output_0.uv_0 = input_0.uv_1;

    output_0.position_0 = float4(input_0.position_1, 1.0f);

    return output_0;
}

