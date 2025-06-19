#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 17 "Prefilter.slang"
struct PrefilterUBO_0
{
    float roughness_0;
};

cbuffer roughnessUBO_0 : register(b1)
{
    PrefilterUBO_0 roughnessUBO_0;
}

#line 12
TextureCube<float4 > environmentMap_0 : register(t0, space1);


#line 11
SamplerState smp_0 : register(s0, space1);


#line 39
float RadicalInverse_VdC_0(uint bits_0)
{
    uint _S1 = (bits_0 << 16U) | (bits_0 >> 16U);
    uint _S2 = ((_S1 & 1431655765U) << 1U) | ((_S1 & 2863311530U) >> 1U);
    uint _S3 = ((_S2 & 858993459U) << 2U) | ((_S2 & 3435973836U) >> 2U);
    uint _S4 = ((_S3 & 252645135U) << 4U) | ((_S3 & 4042322160U) >> 4U);

    return float(((_S4 & 16711935U) << 8U) | ((_S4 & 4278255360U) >> 8U)) * 2.32830643653869629e-10f;
}

float2 Hammersley_0(uint i_0, uint N_0)
{
    return float2(float(i_0) / float(N_0), RadicalInverse_VdC_0(i_0));
}

float3 ImportanceSampleGGX_0(float2 Xi_0, float3 N_1, float alpha_0)
{
    float phi_0 = 6.28318548202514648f * Xi_0.x;
    float _S5 = Xi_0.y;

#line 57
    float cosTheta_0 = sqrt((1.0f - _S5) / (1.0f + (alpha_0 * alpha_0 - 1.0f) * _S5));
    float sinTheta_0 = sqrt(1.0f - cosTheta_0 * cosTheta_0);

    float3 H_0;
    H_0[int(0)] = cos(phi_0) * sinTheta_0;
    H_0[int(1)] = sin(phi_0) * sinTheta_0;
    H_0[int(2)] = cosTheta_0;

#line 63
    float3 up_0;

    if((abs(N_1.z)) < 0.99900001287460327f)
    {

#line 65
        up_0 = float3(0.0f, 0.0f, 1.0f);

#line 65
    }
    else
    {

#line 65
        up_0 = float3(1.0f, 0.0f, 0.0f);

#line 65
    }
    float3 tangent_0 = normalize(cross(up_0, N_1));



    return normalize(tangent_0 * H_0.x + cross(N_1, tangent_0) * H_0.y + N_1 * H_0.z);
}


#line 28
float DistributionGGX_0(float alpha_1, float3 N_2, float3 H_1)
{
    float numerator_0 = pow(alpha_1, 2.0f);

#line 36
    return numerator_0 / max(3.14159274101257324f * pow(pow(max(dot(N_2, H_1), 0.0f), 2.0f) * (numerator_0 - 1.0f) + 1.0f, 2.0f), 9.99999997475242708e-07f);
}


#line 3
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float3 o_position_0 : TEXCOORD0;
};


#line 74
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{
    float3 N_3 = normalize(float3(input_0.o_position_0.x, input_0.o_position_0.y, input_0.o_position_0.z));

#line 81
    float3 _S6 = (float3)0.0f;

#line 81
    uint i_1 = 0U;

#line 81
    float3 prefilteredColor_0 = _S6;

#line 81
    float totalWeight_0 = 0.0f;


    for(;;)
    {

#line 84
        if(i_1 < 1024U)
        {
        }
        else
        {

#line 84
            break;
        }

        float3 H_2 = ImportanceSampleGGX_0(Hammersley_0(i_1, 1024U), N_3, roughnessUBO_0.roughness_0 * roughnessUBO_0.roughness_0);
        float _S7 = dot(N_3, H_2);

#line 88
        float3 L_0 = normalize(2.0f * _S7 * H_2 - N_3);

        float NdotL_0 = max(dot(N_3, L_0), 0.0f);
        if(NdotL_0 > 0.0f)
        {

#line 100
            float saSample_0 = 1.0f / (1024.0f * (DistributionGGX_0(roughnessUBO_0.roughness_0 * roughnessUBO_0.roughness_0, N_3, H_2) * max(_S7, 0.0f) / (4.0f * max(dot(H_2, N_3), 0.0f)) + 0.00009999999747379f) + 0.00009999999747379f);

#line 100
            float mipLevel_0;

            if((roughnessUBO_0.roughness_0) == 0.0f)
            {

#line 102
                mipLevel_0 = 0.0f;

#line 102
            }
            else
            {

#line 102
                mipLevel_0 = 0.5f * log2(saSample_0 / 4.99342718285333831e-07f);

#line 102
            }

#line 109
            float totalWeight_1 = totalWeight_0 + NdotL_0;

#line 109
            prefilteredColor_0 = prefilteredColor_0 + environmentMap_0.SampleLevel(smp_0, L_0, mipLevel_0).xyz * NdotL_0;

#line 109
            totalWeight_0 = totalWeight_1;

#line 91
        }

#line 84
        i_1 = i_1 + 1U;

#line 84
    }

#line 115
    return float4(prefilteredColor_0 / totalWeight_0, 1.0f);
}

