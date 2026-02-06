#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 1 "slang/ToneMapping.slang"
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


VSOutput_0 vertexMain(uint id_0 : SV_VertexID)
{
    VSOutput_0 output_0;
    float2 _S1 = float2(float((id_0 << int(1)) & 2U), float(id_0 & 2U));

#line 11
    output_0.uv_0 = _S1;
    output_0.position_0 = float4(_S1 * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output_0;
}

