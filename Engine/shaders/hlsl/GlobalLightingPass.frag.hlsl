#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 65 "slang/GlobalLightingPass.slang"
Texture2D<float4 > gAlbedo_0 : register(t0, space1);



SamplerState gSampler_0 : register(s0, space1);


#line 50
struct PushConstants_0
{
    float4x4 lightViewProjection_0;
    float3 viewPosition_0;
    int meshletVisualization_0;
    int activeDirectionalLights_0;
    int useShadow_0;
    float orthoSize_0;
};

cbuffer pushConstants_0 : register(b1)
{
    PushConstants_0 pushConstants_0;
}


Texture2D<float4 > gNormal_0 : register(t1, space1);


#line 67
Texture2D<float4 > gPosition_0 : register(t2, space1);


#line 68
Texture2D<float4 > gMaterial_0 : register(t3, space1);


#line 80
Texture2D<float4 > shadowMap_0 : register(t3, space2);


#line 81
SamplerState iblSmp_0 : register(s1, space2);


#line 35
struct fDirectionalLight_0
{
    float3 lightDirection_0;
    float ambientStrength_0;
    float3 lightColor_0;
    float specularStrength_0;
    float intensity_0;
};

struct fDirectionalLightList_0
{
    fDirectionalLight_0  lights_0[int(500)];
};


#line 48
cbuffer directionalLightList_0 : register(b0)
{
    fDirectionalLightList_0 directionalLightList_0;
}

#line 72
TextureCube<float4 > irradianceMap_0 : register(t0, space2);


#line 73
TextureCube<float4 > prefilterMap_0 : register(t1, space2);


#line 74
Texture2D<float4 > brdfLUT_0 : register(t2, space2);


#line 168
struct MainVectors_0
{
    float3 worldPosition_0;
    float3 albedo_0;
    float4 material_0;
    float3 V_0;
    float3 N_0;
    float3 F0_0;
};


#line 168
MainVectors_0 MainVectors_x24init_0(float3 worldPosition_1, float3 albedo_1, float4 material_1, float3 V_1, float3 N_1, float3 F0_1)
{

#line 168
    MainVectors_0 _S1;

    _S1.worldPosition_0 = worldPosition_1;
    _S1.albedo_0 = albedo_1;
    _S1.material_0 = material_1;
    _S1.V_0 = V_1;
    _S1.N_0 = N_1;
    _S1.F0_0 = F0_1;

#line 168
    return _S1;
}


#line 1 "slang/Shadow.slang"
float Hamburger4MSM_0(float4 b_0, float z_f_0)
{

    float4 b_prime_0 = 0.99900001287460327f * b_0 + 0.00100000004749745f * float4(0.5f, 0.5f, 0.5f, 0.5f);

    float _S2 = b_prime_0.y;

#line 6
    float _S3 = b_prime_0.x;

#line 6
    float L22_0 = sqrt(max(_S2 - _S3 * _S3, 0.00000999999974738f));
    float L32_0 = (b_prime_0.z - _S2 * _S3) / L22_0;
    float L33_0 = sqrt(max(b_prime_0.w - _S2 * _S2 - L32_0 * L32_0, 0.00000999999974738f));


    float y2_0 = (z_f_0 - _S3) / L22_0;


    float c3_0 = (z_f_0 * z_f_0 - _S2 - L32_0 * y2_0) / L33_0 / L33_0;
    float c2_0 = (y2_0 - L32_0 * c3_0) / L22_0;



    float sqrtDisc_0 = sqrt(max(0.0f, c2_0 * c2_0 - 4.0f * c3_0 * (1.0f - _S3 * c2_0 - _S2 * c3_0)));

    float _S4 = - c2_0;

#line 21
    float _S5 = 2.0f * c3_0;

#line 21
    float rootA_0 = (_S4 - sqrtDisc_0) / _S5;
    float rootB_0 = (_S4 + sqrtDisc_0) / _S5;

    float z2_0 = min(rootA_0, rootB_0);
    float z3_0 = max(rootA_0, rootB_0);

#line 25
    float G_0;


    if(z_f_0 <= z2_0)
    {

#line 28
        G_0 = 0.0f;

#line 28
    }
    else
    {

#line 29
        if(z_f_0 <= z3_0)
        {

#line 29
            G_0 = (z_f_0 * z3_0 - _S3 * (z_f_0 + z3_0) + _S2) / ((z3_0 - z2_0) * (z_f_0 - z2_0));

#line 29
        }
        else
        {

#line 29
            G_0 = 1.0f - (z2_0 * z3_0 - _S3 * (z2_0 + z3_0) + _S2) / ((z_f_0 - z2_0) * (z_f_0 - z3_0));

#line 29
        }

#line 28
    }



    return G_0;
}


#line 158 "slang/GlobalLightingPass.slang"
float3 F_0(float3 F0_2, float3 V_2, float3 H_0)
{
    return F0_2 + ((float3)1.0f - F0_2) * pow(1.0f - max(dot(V_2, H_0), 0.0f), 5.0f);
}


#line 128
float D_0(float alpha_0, float3 N_2, float3 H_1)
{
    float numerator_0 = pow(alpha_0, 2.0f);

#line 136
    return numerator_0 / max(3.14159274101257324f * pow(pow(max(dot(N_2, H_1), 0.0f), 2.0f) * (numerator_0 - 1.0f) + 1.0f, 2.0f), 9.99999997475242708e-07f);
}


float G1_0(float alpha_1, float3 N_3, float3 X_0)
{
    float numerator_1 = max(dot(N_3, X_0), 0.0f);

    float k_0 = alpha_1 / 2.0f;



    return numerator_1 / max(numerator_1 * (1.0f - k_0) + k_0, 9.99999997475242708e-07f);
}


float G_1(float alpha_2, float3 N_4, float3 V_3, float3 L_0)
{
    return G1_0(alpha_2, N_4, V_3) * G1_0(alpha_2, N_4, L_0);
}


#line 179
float3 PBR_0(MainVectors_0 mainVectors_0, float3 lightPosition_0, float3 lightColor_1, int lightIndex_0)
{

    float roughness_0 = mainVectors_0.material_0.y;

#line 187
    float3 L_1 = normalize(- lightPosition_0);



    float3 H_2 = normalize(mainVectors_0.V_0 + L_1);


    float3 Ks_0 = F_0(mainVectors_0.F0_0, mainVectors_0.V_0, H_2);

#line 200
    float alpha_3 = roughness_0 * roughness_0;

    float _S6 = max(dot(L_1, mainVectors_0.N_0), 0.0f);

#line 209
    return ((1.0f - mainVectors_0.material_0.x) * ((float3)1.0f - Ks_0) * (mainVectors_0.albedo_0 / 3.14159274101257324f) + D_0(alpha_3, mainVectors_0.N_0, H_2) * G_1(alpha_3, mainVectors_0.N_0, mainVectors_0.V_0, L_1) * Ks_0 / max(4.0f * max(dot(mainVectors_0.V_0, mainVectors_0.N_0), 0.0f) * _S6, 0.10000000149011612f)) * lightColor_1 * _S6;
}


#line 163
float3 Froughness_0(float3 F0_3, float3 V_4, float3 H_3, float roughness_1)
{
    return F0_3 + (max((float3)(1.0f - roughness_1), F0_3) - F0_3) * pow(clamp(1.0f - max(dot(H_3, V_4), 0.0f), 0.0f, 1.0f), 5.0f);
}


#line 12
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 213
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 213
    VSOutput_0 _S7 = input_0;

    float3 _S8 = (float3)0.0f;

    float4 albedoSample_0 = gAlbedo_0.Sample(gSampler_0, input_0.uv_0);
    if((albedoSample_0.w) < 0.00999999977648258f)
    {

#line 218
        discard;

#line 218
    }
    float3 albedo_2 = albedoSample_0.xyz;
    if((pushConstants_0.meshletVisualization_0) > int(0))
    {

#line 220
        return float4(albedo_2, 1.0f);
    }

#line 221
    float3 normal_0 = gNormal_0.Sample(gSampler_0, _S7.uv_0).xyz;
    float3 worldPosition_2 = gPosition_0.Sample(gSampler_0, _S7.uv_0).xyz;
    float4 material_2 = gMaterial_0.Sample(gSampler_0, _S7.uv_0);

    float metallic_0 = material_2.x;
    float roughness_2 = material_2.y;



    float3 F0_4 = lerp((float3)0.03999999910593033f, albedo_2, (float3)metallic_0);
    float3 V_5 = normalize(pushConstants_0.viewPosition_0 - worldPosition_2);
    float3 N_5 = normalize(normal_0);
    MainVectors_0 _S9 = MainVectors_x24init_0(worldPosition_2, albedo_2, material_2, V_5, N_5, F0_4);

#line 233
    float shadow_0;

#line 246
    if((pushConstants_0.useShadow_0) > int(0))
    {



        float4 lightSpacePosition_0 = mul(pushConstants_0.lightViewProjection_0, float4(worldPosition_2 + normal_0 * 0.05000000074505806f, 1.0f));
        float3 projectionCoords_0 = lightSpacePosition_0.xyz / lightSpacePosition_0.w;


        float2 shadowUV_0;
        shadowUV_0[int(0)] = projectionCoords_0.x * 0.5f + 0.5f;

        shadowUV_0[int(1)] = - projectionCoords_0.y * 0.5f + 0.5f;

#line 264
        float z_f_1 = projectionCoords_0.z;

#line 264
        shadow_0 = Hamburger4MSM_0(shadowMap_0.Sample(iblSmp_0, shadowUV_0), z_f_1) * (step(0.0f, shadowUV_0.x) * step(shadowUV_0.x, 1.0f) * step(0.0f, shadowUV_0.y) * step(shadowUV_0.y, 1.0f) * step(0.0f, z_f_1) * step(z_f_1, 1.0f));

#line 246
    }
    else
    {

#line 246
        shadow_0 = 0.0f;

#line 246
    }

#line 246
    int l_0 = int(0);

#line 246
    float3 resultColor_0 = _S8;

#line 286
    for(;;)
    {

#line 286
        if(l_0 < (pushConstants_0.activeDirectionalLights_0))
        {
        }
        else
        {

#line 286
            break;
        }


        float3 resultColor_1 = resultColor_0 + PBR_0(_S9, directionalLightList_0.lights_0[l_0].lightDirection_0, directionalLightList_0.lights_0[l_0].lightColor_0 * directionalLightList_0.lights_0[l_0].intensity_0, l_0) * (1.0f - shadow_0);

#line 286
        l_0 = l_0 + int(1);

#line 286
        resultColor_0 = resultColor_1;

#line 286
    }

#line 295
    float3 F_1 = Froughness_0(F0_4, V_5, N_5, roughness_2);

#line 309
    float2 brdf_0 = brdfLUT_0.Sample(iblSmp_0, float2(max(dot(N_5, V_5), 0.0f), roughness_2)).xy;

#line 331
    return float4((1.0f - metallic_0) * ((float3)1.0f - F_1) * (irradianceMap_0.Sample(iblSmp_0, N_5).xyz * albedo_2) + prefilterMap_0.SampleLevel(iblSmp_0, reflect(- V_5, N_5), roughness_2 * 4.0f).xyz * (F_1 * brdf_0.x + brdf_0.y) + resultColor_0, 1.0f);
}

