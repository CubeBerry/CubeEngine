#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 15 "slang/Normal3D.slang"
struct vMatrix_0
{
    float4x4 model_0;
    float4x4 transposeInverseModel_0;
    float4x4 view_0;
    float4x4 projection_0;
    float4x4 decode_0;
    float4 color_0;
    float4 viewPosition_0;
    float4x4  finalBones_0[int(128)];
};



cbuffer matrix_0 : register(b0)
{
    vMatrix_0 matrix_0;
}

#line 31
struct PushConstants_0
{
    float4x4 modelToNDC_0;
};

cbuffer modelToNDC_1 : register(b0, space1)
{
    PushConstants_0 modelToNDC_1;
}

#line 10
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
};


#line 3
struct VSInput_0
{
    float3 position_1 : POSITION0;
    int4 boneIDs_0 : BLENDINDICES0;
    float4 weights_0 : BLENDWEIGHTS0;
};


#line 42
VSOutput_0 vertexMain(VSInput_0 input_0)
{

#line 42
    VSInput_0 _S1 = input_0;

    VSOutput_0 output_0;


    float totalWeight_0 = input_0.weights_0[int(0)] + input_0.weights_0[int(1)] + input_0.weights_0[int(2)] + input_0.weights_0[int(3)];

#line 47
    float4 skinnedPos_0;


    if(totalWeight_0 > 0.00999999977648258f)
    {
        float4 _S2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float _S3 = 1.0f / totalWeight_0;

#line 53
        int i_0 = int(0);

#line 53
        skinnedPos_0 = _S2;

        for(;;)
        {

#line 55
            if(i_0 < int(4))
            {
            }
            else
            {

#line 55
                break;
            }

#line 55
            int _S4 = i_0;

#line 55
            bool _S5;

            if((_S1.boneIDs_0[i_0]) < int(0))
            {

#line 57
                _S5 = true;

#line 57
            }
            else
            {

#line 57
                _S5 = (_S1.boneIDs_0[_S4]) >= int(128);

#line 57
            }

#line 57
            if(_S5)
            {

#line 58
                i_0 = i_0 + int(1);

#line 55
                continue;
            }

#line 55
            int _S6 = i_0;



            if((_S1.weights_0[i_0]) <= 0.0f)
            {

#line 60
                i_0 = i_0 + int(1);

#line 55
                continue;
            }

#line 55
            skinnedPos_0 = skinnedPos_0 + mul(matrix_0.finalBones_0[_S1.boneIDs_0[_S4]], float4(_S1.position_1, 1.0f)) * (_S1.weights_0[_S6] * _S3);

#line 55
            i_0 = i_0 + int(1);

#line 55
        }

#line 50
    }
    else
    {

#line 50
        skinnedPos_0 = float4(_S1.position_1, 1.0f);

#line 50
    }

#line 74
    output_0.position_0 = mul(modelToNDC_1.modelToNDC_0, skinnedPos_0);

    return output_0;
}

