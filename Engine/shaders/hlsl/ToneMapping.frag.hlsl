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


struct PushConstants_0
{
    float exposure_0;
};


#line 39
cbuffer pushConstants_0 : register(b0)
{
    PushConstants_0 pushConstants_0;
}

#line 23
float3 FilmicToneMapping_0(float3 color_0)
{

#line 29
    return clamp(color_0 * (2.50999999046325684f * color_0 + 0.02999999932944775f) / (color_0 * (2.43000006675720215f * color_0 + 0.5899999737739563f) + 0.14000000059604645f), (float3)0.0f, (float3)1.0f);
}


#line 1
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 42
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 54
    return float4(pow(FilmicToneMapping_0(gTexture_0.Sample(gSampler_0, input_0.uv_0).xyz * pushConstants_0.exposure_0), (float3)0.45454543828964233f), 1.0f);
}

