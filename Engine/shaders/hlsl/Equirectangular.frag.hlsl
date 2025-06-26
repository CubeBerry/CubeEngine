#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 12 "Equirectangular.slang"
Texture2D<float4 > equirectangularMap_0 : register(t0, space1);


#line 11
SamplerState smp_0 : register(s0, space1);


#line 17
float2 SampleSphericalMap_0(float3 v_0)
{



    return float2(atan2(v_0.z, v_0.x), asin(v_0.y)) * float2(0.1590999960899353f, 0.31830000877380371f) + 0.5f;
}


float3 ReinhardToneMapping_0(float3 color_0)
{
    return color_0 / (color_0 + (float3)1.0f);
}


#line 1
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 o_position_0 : TEXCOORD0;
};


#line 42
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 56
    return float4(ReinhardToneMapping_0(equirectangularMap_0.Sample(smp_0, SampleSphericalMap_0(normalize(input_0.o_position_0))).xyz), 1.0f);
}

