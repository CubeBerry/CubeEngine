#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 12 "Skybox.slang"
struct WorldToNDC_0
{
    float4x4 view_0;
    float4x4 projection_0;
};

cbuffer worldToNDC_0 : register(b0)
{
    WorldToNDC_0 worldToNDC_0;
}

#line 6
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 uvw_0 : TEXCOORD0;
};


#line 1
struct VSInput_0
{
    float3 position_1 : POSITION0;
};


#line 24
VSOutput_0 vertexMain(VSInput_0 input_0)
{
    VSOutput_0 output_0;

    output_0.uvw_0 = input_0.position_1;

#line 38
    output_0.position_0 = mul(worldToNDC_0.projection_0, mul(float4x4(float4(worldToNDC_0.view_0[int(0)].xyz, 0.0f), float4(worldToNDC_0.view_0[int(1)].xyz, 0.0f), float4(worldToNDC_0.view_0[int(2)].xyz, 0.0f), (float4)0.0f), float4(input_0.position_1, 1.0f))).xyww;

    return output_0;
}

