#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 12 "slang/Irradiance.slang"
TextureCube<float4 > environmentMap_0 : register(t0, space1);


#line 11
SamplerState smp_0 : register(s0, space1);


#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 uvw_0 : TEXCOORD0;
};


#line 18
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{
    float3 N_0 = normalize(float3(input_0.uvw_0.x, input_0.uvw_0.y, input_0.uvw_0.z));
    float3 _S1 = (float3)0.0f;

#line 21
    float3 up_0;


    if((abs(N_0.y)) < 0.99900001287460327f)
    {

#line 24
        up_0 = float3(0.0f, 1.0f, 0.0f);

#line 24
    }
    else
    {

#line 24
        up_0 = float3(0.0f, 0.0f, 1.0f);

#line 24
    }
    float3 right_0 = normalize(cross(up_0, N_0));
    float3 _S2 = normalize(cross(N_0, right_0));

#line 26
    float phi_0 = 0.0f;

#line 26
    float3 irradiance_0 = _S1;

#line 26
    float nrSamples_0 = 0.0f;



    for(;;)
    {

#line 30
        if(phi_0 < 6.28318548202514648f)
        {
        }
        else
        {

#line 30
            break;
        }

#line 30
        float theta_0 = 0.0f;

        for(;;)
        {

#line 32
            if(theta_0 < 1.57079637050628662f)
            {
            }
            else
            {

#line 32
                break;
            }

            float _S3 = sin(theta_0);

#line 35
            float _S4 = cos(theta_0);

#line 40
            float3 irradiance_1 = irradiance_0 + environmentMap_0.Sample(smp_0, _S3 * cos(phi_0) * right_0 + _S3 * sin(phi_0) * _S2 + _S4 * N_0).xyz * _S4 * _S3;



            float _S5 = nrSamples_0 + 1.0f;

#line 32
            theta_0 = theta_0 + 0.02500000037252903f;

#line 32
            irradiance_0 = irradiance_1;

#line 32
            nrSamples_0 = _S5;

#line 32
        }

#line 30
        phi_0 = phi_0 + 0.02500000037252903f;

#line 30
    }

#line 49
    return float4(3.14159274101257324f * irradiance_0 * (1.0f / nrSamples_0), 1.0f);
}

