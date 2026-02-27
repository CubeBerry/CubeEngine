#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 6 "slang/ConvolutionBlur.slang"
struct BlurParams_0
{
    float4  weights_0[int(101)];
    int blurWidth_0;
    int isVertical_0;
};

cbuffer blurParams_0 : register(b0)
{
    BlurParams_0 blurParams_0;
}


Texture2D<float4 > input_0 : register(t0);


#line 20
RWTexture2D<float4 > output_0 : register(u0);


#line 26
static groupshared float4  v_0[int(229)];


[numthreads(128, 1, 1)]
void computeMain(uint3 GroupID_0 : SV_GroupID, uint3 GTid_0 : SV_GroupThreadID)
{

#line 29
    uint3 _S1 = GroupID_0;

#line 34
    int w_0 = blurParams_0.blurWidth_0;
    uint lpos_0 = GTid_0.x;

#line 35
    int2 gpos_0;
    if((blurParams_0.isVertical_0) == int(0))
    {

#line 36
        gpos_0 = int2(int(_S1.x * 128U + lpos_0), int(_S1.y));

#line 36
    }
    else
    {

#line 36
        gpos_0 = int2(int(_S1.x), int(_S1.y * 128U + lpos_0));

#line 36
    }

#line 36
    int2 dir_0;
    if((blurParams_0.isVertical_0) == int(0))
    {

#line 37
        dir_0 = int2(int(1), int(0));

#line 37
    }
    else
    {

#line 37
        dir_0 = int2(int(0), int(1));

#line 37
    }


    v_0[lpos_0] = input_0[uint2(gpos_0 - dir_0 * w_0)];

    int _S2 = int(2) * w_0;

#line 42
    if(lpos_0 < uint(_S2))
    {

        v_0[lpos_0 + 128U] = input_0[uint2(gpos_0 + dir_0 * (int(128) - w_0))];

#line 42
    }

#line 48
    GroupMemoryBarrierWithGroupSync();

    float4 _S3 = (float4)0.0f;

#line 50
    int i_0 = int(0);

#line 50
    float4 sum_0 = _S3;
    for(;;)
    {

#line 51
        if(i_0 <= _S2)
        {
        }
        else
        {

#line 51
            break;
        }
        float4 sum_1 = sum_0 + v_0[lpos_0 + uint(i_0)] * blurParams_0.weights_0[i_0].x;

#line 51
        i_0 = i_0 + int(1);

#line 51
        sum_0 = sum_1;

#line 51
    }



    uint width_0;

#line 55
    uint height_0;
    output_0.GetDimensions(width_0, height_0);

#line 56
    bool _S4;
    if(uint(gpos_0.x) < width_0)
    {

#line 57
        _S4 = uint(gpos_0.y) < height_0;

#line 57
    }
    else
    {

#line 57
        _S4 = false;

#line 57
    }

#line 57
    if(_S4)
    {
        output_0[uint2(gpos_0)] = sum_0;

#line 57
    }



    return;
}

