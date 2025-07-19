#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 18 "2D.slang"
struct vMatrix_0
{
    float4x4 model_0;
    float4x4 view_0;
    float4x4 projection_0;
    float4x4 decode_0;
    float4 color_0;
    float3 frameSize_0;
    float isTex_0;
    float3 texelPos_0;
    float isTexel_0;
};


#line 30
cbuffer matrix_0 : register(b0)
{
    vMatrix_0 matrix_0;
}

#line 10
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
    float4 color_1 : COLOR0;
    float isTex_1 : TEXCOORD1;
};


#line 5
struct VSInput_0
{
    uint position_1 : POSITION0;
};


#line 33
VSOutput_0 vertexMain(VSInput_0 input_0)
{
    VSOutput_0 output_0;

#line 40
    float2 decoded_position_0 = mul(matrix_0.decode_0, float4(float2(float((input_0.position_1) & 65535U), float(((input_0.position_1) >> int(16)) & 65535U)), 0.0f, 1.0f)).xy;

#line 48
    if((matrix_0.isTexel_0) == 1.0f)
    {
        float _S1 = decoded_position_0.x + 1.0f;

#line 50
        output_0.uv_0[int(0)] = lerp(_S1 / 2.0f, _S1 / 2.0f * matrix_0.frameSize_0.x + matrix_0.texelPos_0.x, matrix_0.isTexel_0);
        float _S2 = decoded_position_0.y + 1.0f;

#line 51
        output_0.uv_0[int(1)] = lerp(_S2 / 2.0f, _S2 / 2.0f * matrix_0.frameSize_0.y + matrix_0.texelPos_0.y, matrix_0.isTexel_0);

#line 48
    }
    else
    {

#line 55
        output_0.uv_0[int(0)] = (decoded_position_0.x + 1.0f) / 2.0f;
        output_0.uv_0[int(1)] = (decoded_position_0.y + 1.0f) / 2.0f;

#line 48
    }

#line 59
    output_0.isTex_1 = matrix_0.isTex_0;
    output_0.color_1 = matrix_0.color_0;

    output_0.position_0 = mul(matrix_0.projection_0, mul(matrix_0.view_0, mul(matrix_0.model_0, float4(decoded_position_0, 0.0f, 1.0f))));

    return output_0;
}

