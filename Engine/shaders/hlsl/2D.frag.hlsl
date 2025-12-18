#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 67 "slang/2D.slang"
struct fMatrix_0
{
    int texIndex_0;
};


#line 71
cbuffer f_matrix_0 : register(b1)
{
    fMatrix_0 f_matrix_0;
}
Texture2D<float4 >  tex_0[int(500)] : register(t0, space1);


#line 74
SamplerState smp_0 : register(s0, space1);


#line 10
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
    float4 color_0 : COLOR0;
    float isTex_0 : TEXCOORD1;
};


#line 81
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 81
    VSOutput_0 _S1 = input_0;


    float4 texColor_0 = lerp(input_0.color_0, tex_0[f_matrix_0.texIndex_0].Sample(smp_0, input_0.uv_0), (float4)input_0.isTex_0);

#line 89
    if((texColor_0.w) < 0.5f)
    {

#line 89
        discard;

#line 89
    }

#line 100
    return _S1.color_0 * texColor_0;
}

