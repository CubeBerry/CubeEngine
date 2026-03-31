#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 37 "slang/3D.slang"
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

#line 22
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
    float4 color_1 : COLOR0;
    int tex_sub_index_0 : TEXCOORD1;
    int4 boneIDs_0 : BLENDINDICES0;
    float4 weights_0 : BLENDWEIGHTS0;
    float3 normal_0 : NORMAL0;
    float3 fragmentPosition_0 : TEXCOORD2;
    float3 viewPosition_1 : TEXCOORD3;
    bool meshletVisualization_0 : TEXCOORD4;
};


#line 12
struct VSInput_0
{
    uint position_1 : POSITION0;
    float3 normal_1 : NORMAL0;
    float2 uv_1 : TEXCOORD0;
    int tex_sub_index_1 : TEXCOORD1;
    int4 boneIDs_1 : BLENDINDICES0;
    float4 weights_1 : BLENDWEIGHTS0;
};


#line 55
VSOutput_0 vertexMain(VSInput_0 input_0)
{

#line 55
    VSInput_0 _S1 = input_0;

#line 63
    float3 decoded_position_0 = mul(matrix_0.decode_0, float4(float3(float((input_0.position_1) & 2047U), float(((input_0.position_1) >> int(11)) & 2047U), float(((input_0.position_1) >> int(22)) & 1023U)), 1.0f)).xyz;

#line 57
    VSOutput_0 output_0;

#line 65
    output_0.uv_0 = input_0.uv_1;
    output_0.color_1 = matrix_0.color_0;
    output_0.tex_sub_index_0 = input_0.tex_sub_index_1;

    output_0.boneIDs_0 = input_0.boneIDs_1;
    output_0.weights_0 = input_0.weights_1;
    output_0.meshletVisualization_0 = false;


    float4 _S2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 _S3 = float3(0.0f, 0.0f, 0.0f);

    float totalWeight_0 = input_0.weights_1[int(0)] + input_0.weights_1[int(1)] + input_0.weights_1[int(2)] + input_0.weights_1[int(3)];

#line 77
    float4 totalPosition_0;

#line 77
    float3 totalNormal_0;

    if(totalWeight_0 > 0.00999999977648258f)
    {

        float _S4 = 1.0f / totalWeight_0;

#line 82
        int i_0 = int(0);

#line 82
        totalPosition_0 = _S2;

#line 82
        totalNormal_0 = _S3;

        for(;;)
        {

#line 84
            if(i_0 < int(4))
            {
            }
            else
            {

#line 84
                break;
            }

#line 84
            int _S5 = i_0;

#line 84
            bool _S6;

            if((_S1.boneIDs_1[i_0]) < int(0))
            {

#line 86
                _S6 = true;

#line 86
            }
            else
            {

#line 86
                _S6 = (_S1.boneIDs_1[_S5]) >= int(128);

#line 86
            }

#line 86
            if(_S6)
            {

#line 87
                i_0 = i_0 + int(1);

#line 84
                continue;
            }

#line 84
            int _S7 = i_0;



            if((_S1.weights_1[i_0]) <= 0.0f)
            {

#line 89
                i_0 = i_0 + int(1);

#line 84
                continue;
            }

#line 91
            float normalizedWeight_0 = _S1.weights_1[_S7] * _S4;

#line 97
            float3 totalNormal_1 = totalNormal_0 + mul(float3x3(matrix_0.finalBones_0[_S1.boneIDs_1[_S5]][int(0)].xyz, matrix_0.finalBones_0[_S1.boneIDs_1[_S5]][int(1)].xyz, matrix_0.finalBones_0[_S1.boneIDs_1[_S5]][int(2)].xyz), _S1.normal_1) * normalizedWeight_0;

#line 97
            totalPosition_0 = totalPosition_0 + mul(matrix_0.finalBones_0[_S1.boneIDs_1[_S5]], float4(decoded_position_0, 1.0f)) * normalizedWeight_0;

#line 97
            totalNormal_0 = totalNormal_1;

#line 84
            i_0 = i_0 + int(1);

#line 84
        }

#line 79
    }
    else
    {

#line 103
        float4 _S8 = float4(decoded_position_0, 1.0f);

#line 103
        totalNormal_0 = _S1.normal_1;

#line 103
        totalPosition_0 = _S8;

#line 79
    }

#line 110
    output_0.normal_0 = mul(float3x3(matrix_0.transposeInverseModel_0[int(0)].xyz, matrix_0.transposeInverseModel_0[int(1)].xyz, matrix_0.transposeInverseModel_0[int(2)].xyz), totalNormal_0);
    output_0.fragmentPosition_0 = mul(matrix_0.model_0, totalPosition_0).xyz;
    output_0.viewPosition_1 = matrix_0.viewPosition_0.xyz;


    output_0.position_0 = mul(matrix_0.projection_0, mul(matrix_0.view_0, mul(matrix_0.model_0, totalPosition_0)));

    return output_0;
}

