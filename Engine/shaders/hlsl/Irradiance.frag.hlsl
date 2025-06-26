#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 12 "Irradiance.slang"
TextureCube<float4 > environmentMap_0 : register(t0, space1);


#line 11
SamplerState smp_0 : register(s0, space1);


#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 o_position_0 : TEXCOORD0;
};


#line 18
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{
    float3 N_0 = normalize(float3(input_0.o_position_0.x, input_0.o_position_0.y, input_0.o_position_0.z));
    float3 _S1 = (float3)0.0f;


    float3 right_0 = normalize(cross(float3(0.0f, 1.0f, 0.0f), N_0));
    float3 _S2 = normalize(cross(N_0, right_0));

#line 25
    float phi_0 = 0.0f;

#line 25
    float3 irradiance_0 = _S1;

#line 25
    float nrSamples_0 = 0.0f;



    for(;;)
    {

#line 29
        if(phi_0 < 6.28318548202514648f)
        {
        }
        else
        {

#line 29
            break;
        }

#line 29
        float theta_0 = 0.0f;

        for(;;)
        {

#line 31
            if(theta_0 < 1.57079637050628662f)
            {
            }
            else
            {

#line 31
                break;
            }

#line 39
            float3 irradiance_1 = irradiance_0 + environmentMap_0.Sample(smp_0, sin(theta_0) * cos(phi_0) * right_0 + sin(theta_0) * sin(phi_0) * _S2 + cos(theta_0) * N_0).xyz * cos(theta_0) * sin(theta_0);



            float _S3 = nrSamples_0 + 1.0f;

#line 31
            theta_0 = theta_0 + 0.02500000037252903f;

#line 31
            irradiance_0 = irradiance_1;

#line 31
            nrSamples_0 = _S3;

#line 31
        }

#line 29
        phi_0 = phi_0 + 0.02500000037252903f;

#line 29
    }

#line 48
    return float4(3.14159274101257324f * irradiance_0 * (1.0f / nrSamples_0), 1.0f);
}

