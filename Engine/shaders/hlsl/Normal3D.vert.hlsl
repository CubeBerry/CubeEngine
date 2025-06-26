#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 13 "Normal3D.slang"
struct PushConstants_0
{
    float4x4 modelToNDC_0;
};

cbuffer modelToNDC_1 : register(b0)
{
    PushConstants_0 modelToNDC_1;
}

#line 8
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
};


#line 3
struct VSInput_0
{
    float3 position_1 : POSITION0;
};


#line 24
VSOutput_0 vertexMain(VSInput_0 input_0)
{
    VSOutput_0 output_0;

    output_0.position_0 = mul(modelToNDC_1.modelToNDC_0, float4(input_0.position_1, 1.0f));

    return output_0;
}

