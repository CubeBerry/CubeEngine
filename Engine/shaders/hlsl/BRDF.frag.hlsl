#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 27 "BRDF.slang"
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

#line 45
    float cosTheta_0 = sqrt((1.0f - _S5) / (1.0f + (alpha_0 * alpha_0 - 1.0f) * _S5));
    float sinTheta_0 = sqrt(1.0f - cosTheta_0 * cosTheta_0);

    float3 H_0;
    H_0[int(0)] = cos(phi_0) * sinTheta_0;
    H_0[int(1)] = sin(phi_0) * sinTheta_0;
    H_0[int(2)] = cosTheta_0;

#line 51
    float3 up_0;

    if((abs(N_1.z)) < 0.99900001287460327f)
    {

#line 53
        up_0 = float3(0.0f, 0.0f, 1.0f);

#line 53
    }
    else
    {

#line 53
        up_0 = float3(1.0f, 0.0f, 0.0f);

#line 53
    }
    float3 tangent_0 = normalize(cross(up_0, N_1));



    return normalize(tangent_0 * H_0.x + cross(N_1, tangent_0) * H_0.y + N_1 * H_0.z);
}


float G1_0(float alpha_1, float3 N_2, float3 X_0)
{
    float numerator_0 = max(dot(N_2, X_0), 0.0f);

    float k_0 = alpha_1 / 2.0f;



    return numerator_0 / max(numerator_0 * (1.0f - k_0) + k_0, 9.99999997475242708e-07f);
}


float G_0(float alpha_2, float3 N_3, float3 V_0, float3 L_0)
{
    return G1_0(alpha_2, N_3, V_0) * G1_0(alpha_2, N_3, L_0);
}

float2 IntegrateBRDF_0(float NdotV_0, float roughness_0)
{
    float3 V_1;
    V_1[int(0)] = sqrt(1.0f - NdotV_0 * NdotV_0);
    V_1[int(1)] = 0.0f;
    V_1[int(2)] = NdotV_0;

#line 89
    float3 _S6 = float3(0.0f, 0.0f, 1.0f);

#line 89
    uint i_1 = 0U;

#line 89
    float A_0 = 0.0f;

#line 89
    float B_0 = 0.0f;


    for(;;)
    {

#line 92
        if(i_1 < 1024U)
        {
        }
        else
        {

#line 92
            break;
        }

        float3 H_1 = ImportanceSampleGGX_0(Hammersley_0(i_1, 1024U), _S6, roughness_0);
        float3 L_1 = normalize(2.0f * dot(V_1, H_1) * H_1 - V_1);


        float NdotH_0 = max(H_1.z, 0.0f);
        float VdotH_0 = max(dot(V_1, H_1), 0.0f);

        if((max(L_1.z, 0.0f)) > 0.0f)
        {

            float G_Vis_0 = G_0(roughness_0 * roughness_0, _S6, V_1, L_1) * VdotH_0 / (NdotH_0 * NdotV_0);
            float Fc_0 = pow(1.0f - VdotH_0, 5.0f);


            float B_1 = B_0 + Fc_0 * G_Vis_0;

#line 109
            A_0 = A_0 + (1.0f - Fc_0) * G_Vis_0;

#line 109
            B_0 = B_1;

#line 102
        }

#line 92
        i_1 = i_1 + 1U;

#line 92
    }

#line 114
    return float2(A_0 / 1024.0f, B_0 / 1024.0f);
}


#line 9
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 118
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

    return float4(IntegrateBRDF_0(input_0.uv_0.x, input_0.uv_0.y), 0.0f, 1.0f);
}

