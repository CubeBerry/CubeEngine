#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 8 "Normal3D.slang"
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
};


#line 34
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

