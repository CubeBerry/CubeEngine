#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 8 "Compute.slang"
Texture2DMS<float4 > g_input_0 : register(t0);


#line 9
RWTexture2D<float4 > g_output_0 : register(u0);


#line 16
[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{

#line 18
    uint2 _S1 = dispatchThreadID_0.xy;

    g_output_0[_S1] = float4(1.0 - g_input_0.Load(int2(_S1), int(0)).xyz, 1.0);
    return;
}

