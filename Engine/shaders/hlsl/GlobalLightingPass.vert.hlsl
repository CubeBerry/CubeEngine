#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 10 "slang/GlobalLightingPass.slang"
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


VSOutput_0 vertexMain(uint vertexID_0 : SV_VertexID)
{


    float2 texcoord_0 = float2(float((vertexID_0 << int(1)) & 2U), float(vertexID_0 & 2U));

#line 19
    VSOutput_0 output_0;



    output_0.position_0 = float4(texcoord_0 * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);



    output_0.uv_0 = texcoord_0;

    return output_0;
}

