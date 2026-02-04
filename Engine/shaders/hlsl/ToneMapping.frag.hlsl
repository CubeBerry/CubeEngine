#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 33 "slang/ToneMapping.slang"
Texture2D<float4 > gTexture_0 : register(t0);


#line 32
SamplerState gSampler_0 : register(s0, space1);


#line 17
float3 ReinhardToneMapping_0(float3 color_0)
{
    return color_0 / (color_0 + (float3)1.0f);
}


#line 1
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 36
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 44
    return float4(pow(ReinhardToneMapping_0(gTexture_0.Sample(gSampler_0, input_0.uv_0).xyz), (float3)0.45454543828964233f), 1.0f);
}

