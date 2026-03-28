#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 17 "slang/Irradiance.slang"
struct SHCoefficients_0
{
    float4  E_lm_0[int(9)];
};


#line 21
cbuffer shBuffer_0 : register(b1)
{
    SHCoefficients_0 shBuffer_0;
}

#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 uvw_0 : TEXCOORD0;
};


#line 58
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{
    float3 N_0 = normalize(float3(input_0.uvw_0.x, input_0.uvw_0.y, input_0.uvw_0.z));
    float x_0 = N_0.x;
    float y_0 = N_0.y;
    float z_0 = N_0.z;


    float _S1 = 0.5f * sqrt(0.9549296498298645f);


    float _S2 = sqrt(4.77464818954467773f);

#line 69
    float _S3 = 0.5f * _S2;

#line 69
    float _S4 = _S3 * x_0;

#line 86
    return float4(shBuffer_0.E_lm_0[int(0)].xyz * (0.5f * sqrt(0.31830987334251404f)) + shBuffer_0.E_lm_0[int(1)].xyz * (_S1 * y_0) + shBuffer_0.E_lm_0[int(2)].xyz * (_S1 * z_0) + shBuffer_0.E_lm_0[int(3)].xyz * (_S1 * x_0) + shBuffer_0.E_lm_0[int(4)].xyz * (_S4 * y_0) + shBuffer_0.E_lm_0[int(5)].xyz * (_S3 * y_0 * z_0) + shBuffer_0.E_lm_0[int(6)].xyz * (0.25f * sqrt(1.59154939651489258f) * (3.0f * z_0 * z_0 - 1.0f)) + shBuffer_0.E_lm_0[int(7)].xyz * (_S4 * z_0) + shBuffer_0.E_lm_0[int(8)].xyz * (0.25f * _S2 * (x_0 * x_0 - y_0 * y_0)), 1.0f);
}

