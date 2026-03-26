#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 2 "slang/MipmapGen.slang"
RWTexture2DArray<float4 > dstTexture_0 : register(u0);


struct MipGenConstants_0
{
    float2 texelSize_0;
};


#line 9
cbuffer cb_0 : register(b0)
{
    MipGenConstants_0 cb_0;
}

#line 1
Texture2DArray<float4 > srcTexture_0 : register(t0);

SamplerState LinearClampSampler_0 : register(s0);


#line 12
[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{

#line 12
    uint3 _S1 = dispatchThreadID_0;

    uint dstWidth_0;

#line 14
    uint dstHeight_0;

#line 14
    uint elements_0;
    dstTexture_0.GetDimensions(dstWidth_0, dstHeight_0, elements_0);

#line 15
    bool _S2;

    if((_S1.x) >= dstWidth_0)
    {

#line 17
        _S2 = true;

#line 17
    }
    else
    {

#line 17
        _S2 = (_S1.y) >= dstHeight_0;

#line 17
    }

#line 17
    if(_S2)
    {

#line 17
        _S2 = true;

#line 17
    }
    else
    {

#line 17
        _S2 = (_S1.z) >= elements_0;

#line 17
    }

#line 17
    if(_S2)
    {

#line 17
        return;
    }


    dstTexture_0[_S1] = srcTexture_0.SampleLevel(LinearClampSampler_0, float3((float2(_S1.xy) + 0.5f) * cb_0.texelSize_0, float(_S1.z)), 0.0f);
    return;
}

