#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 9 "slang/Compute.slang"
RWTexture2D<float4 > g_output_0 : register(u0);


#line 16
[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{



    g_output_0[dispatchThreadID_0.xy] = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return;
}

