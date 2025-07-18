#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 3 "BRDF.slang"
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


VSOutput_0 vertexMain(uint id_0 : SV_VertexID)
{

#line 17
    float2  positions_0[int(3)] = { float2(-1.0f, 1.0f), float2(3.0f, 1.0f), float2(-1.0f, -3.0f) };

#line 12
    VSOutput_0 output_0;

#line 32
    output_0.uv_0 = positions_0[id_0] * 0.5f + 0.5f;

    output_0.position_0 = float4(positions_0[id_0], 0.0f, 1.0f);

    return output_0;
}

