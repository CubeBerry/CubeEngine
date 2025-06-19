#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 45 "Skybox.slang"
TextureCube<float4 > skybox_0 : register(t0, space1);


#line 44
SamplerState smp_0 : register(s0, space1);


#line 6
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 uvw_0 : TEXCOORD0;
};


#line 51
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 58
    return float4(skybox_0.Sample(smp_0, input_0.uvw_0).xyz, 1.0f);
}

