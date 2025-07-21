#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 27 "3D.slang"
struct vMatrix_0
{
    float4x4 model_0;
    float4x4 transposeInverseModel_0;
    float4x4 view_0;
    float4x4 projection_0;
    float4x4 decode_0;
    float4 color_0;
    float3 viewPosition_0;
};


cbuffer matrix_0 : register(b0)
{
    vMatrix_0 matrix_0;
}

#line 15
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
    float4 color_1 : COLOR0;
    int tex_sub_index_0 : TEXCOORD1;
    float3 normal_0 : NORMAL0;
    float3 fragmentPosition_0 : LIGHT0;
    float3 viewPosition_1 : LIGHT1;
};


#line 7
struct VSInput_0
{
    uint position_1 : POSITION0;
    float3 normal_1 : NORMAL0;
    float2 uv_1 : TEXCOORD0;
    int tex_sub_index_1 : TEXCOORD1;
};


#line 42
VSOutput_0 vertexMain(VSInput_0 input_0)
{

#line 50
    float3 decoded_position_0 = mul(matrix_0.decode_0, float4(float3(float((input_0.position_1) & 2047U), float(((input_0.position_1) >> int(11)) & 2047U), float(((input_0.position_1) >> int(22)) & 1023U)), 1.0f)).xyz;

#line 44
    VSOutput_0 output_0;

#line 52
    output_0.uv_0 = input_0.uv_1;
    output_0.color_1 = matrix_0.color_0;
    output_0.tex_sub_index_0 = input_0.tex_sub_index_1;


    output_0.normal_0 = mul(float3x3(matrix_0.transposeInverseModel_0[int(0)].xyz, matrix_0.transposeInverseModel_0[int(1)].xyz, matrix_0.transposeInverseModel_0[int(2)].xyz), input_0.normal_1);
    float4 _S1 = float4(decoded_position_0, 1.0f);

#line 58
    output_0.fragmentPosition_0 = mul(matrix_0.model_0, _S1).xyz;
    output_0.viewPosition_1 = matrix_0.viewPosition_0;

    output_0.position_0 = mul(matrix_0.projection_0, mul(matrix_0.view_0, mul(matrix_0.model_0, _S1)));

    return output_0;
}

