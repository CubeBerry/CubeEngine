#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 195 "slang/3D.slang"
struct fMatrix_0
{
    int isTex_0;
    int texIndex_0;
};


#line 200
cbuffer f_matrix_0 : register(b1)
{
    fMatrix_0 f_matrix_0;
}
Texture2D<float4 >  tex_0[int(500)] : register(t0, space1);


#line 203
SamplerState smp_0 : register(s0, space1);


#line 209
struct fMaterial_0
{
    float3 specularColor_0;
    float shininess_0;
    float metallic_0;
    float roughness_0;
};



cbuffer f_material_0 : register(b2)
{
    fMaterial_0 f_material_0;
}

#line 264
struct PushConstants_0
{
    int activeDirectionalLights_0;
    int activePointLights_0;
};

cbuffer pushConstants_0 : register(b5)
{
    PushConstants_0 pushConstants_0;
}

#line 221
struct fDirectionalLight_0
{
    float3 lightDirection_0;
    float ambientStrength_0;
    float3 lightColor_0;
    float specularStrength_0;
};

struct fDirectionalLightList_0
{
    fDirectionalLight_0  lights_0[int(10)];
};


#line 233
cbuffer directionalLightList_0 : register(b3)
{
    fDirectionalLightList_0 directionalLightList_0;
}

#line 235
struct fPointLight_0
{
    float3 lightPosition_0;
    float ambientStrength_1;
    float3 lightColor_1;
    float specularStrength_1;
    float constant_0;
    float linear_0;
    float quadratic_0;
};


struct fPointLightList_0
{
    fPointLight_0  lights_1[int(10)];
};


#line 251
cbuffer pointLightList_0 : register(b4)
{
    fPointLightList_0 pointLightList_0;
}
TextureCube<float4 > irradianceMap_0 : register(t0, space2);


#line 254
SamplerState iblSmp_0 : register(s1, space2);

TextureCube<float4 > prefilterMap_0 : register(t1, space2);


#line 257
Texture2D<float4 > brdfLUT_0 : register(t2, space2);


#line 360
struct MainVectors_0
{
    float3 fragmentPosition_0;
    float3 albedo_0;
    float3 V_0;
    float3 N_0;
    float3 F0_0;
};


#line 360
MainVectors_0 MainVectors_x24init_0(float3 fragmentPosition_1, float3 albedo_1, float3 V_1, float3 N_1, float3 F0_1)
{

#line 360
    MainVectors_0 _S1;

    _S1.fragmentPosition_0 = fragmentPosition_1;
    _S1.albedo_0 = albedo_1;
    _S1.V_0 = V_1;
    _S1.N_0 = N_1;
    _S1.F0_0 = F0_1;

#line 360
    return _S1;
}


#line 350
float3 F_0(float3 F0_2, float3 V_2, float3 H_0)
{
    return F0_2 + ((float3)1.0f - F0_2) * pow(1.0f - max(dot(V_2, H_0), 0.0f), 5.0f);
}


#line 320
float D_0(float alpha_0, float3 N_2, float3 H_1)
{
    float numerator_0 = pow(alpha_0, 2.0f);

#line 328
    return numerator_0 / max(3.14159274101257324f * pow(pow(max(dot(N_2, H_1), 0.0f), 2.0f) * (numerator_0 - 1.0f) + 1.0f, 2.0f), 9.99999997475242708e-07f);
}


float G1_0(float alpha_1, float3 N_3, float3 X_0)
{
    float numerator_1 = max(dot(N_3, X_0), 0.0f);

    float k_0 = alpha_1 / 2.0f;



    return numerator_1 / max(numerator_1 * (1.0f - k_0) + k_0, 9.99999997475242708e-07f);
}


float G_0(float alpha_2, float3 N_4, float3 V_3, float3 L_0)
{
    return G1_0(alpha_2, N_4, V_3) * G1_0(alpha_2, N_4, L_0);
}


#line 370
float3 PBR_0(MainVectors_0 mainVectors_0, float3 lightPosition_1, float3 lightColor_2, bool isPointLight_0, int lightIndex_0)
{

#line 370
    float _S2 = f_material_0.metallic_0;

#line 370
    float _S3 = f_material_0.roughness_0;

#line 370
    float distance_0;

#line 375
    if(isPointLight_0)
    {

#line 375
        distance_0 = length(lightPosition_1 - mainVectors_0.fragmentPosition_0);

#line 375
    }
    else
    {

#line 375
        distance_0 = 1.0f;

#line 375
    }

#line 375
    float attenuation_0;

#line 375
    float3 L_1;



    if(isPointLight_0)
    {

        float _S4 = 1.0f / (pointLightList_0.lights_1[lightIndex_0].constant_0 + pointLightList_0.lights_1[lightIndex_0].linear_0 * distance_0 + pointLightList_0.lights_1[lightIndex_0].quadratic_0 * (distance_0 * distance_0));

#line 382
        L_1 = normalize(lightPosition_1 - mainVectors_0.fragmentPosition_0);

#line 382
        attenuation_0 = _S4;

#line 379
    }
    else
    {

#line 379
        L_1 = normalize(- lightPosition_1);

#line 379
        attenuation_0 = 1.0f;

#line 379
    }

#line 391
    float3 H_2 = normalize(mainVectors_0.V_0 + L_1);


    float3 Ks_0 = F_0(mainVectors_0.F0_0, mainVectors_0.V_0, H_2);

#line 400
    float alpha_3 = _S3 * _S3;

    float _S5 = max(dot(L_1, mainVectors_0.N_0), 0.0f);

#line 409
    return ((1.0f - _S2) * ((float3)1.0f - Ks_0) * (mainVectors_0.albedo_0 / 3.14159274101257324f) + D_0(alpha_3, mainVectors_0.N_0, H_2) * G_0(alpha_3, mainVectors_0.N_0, mainVectors_0.V_0, L_1) * Ks_0 / max(4.0f * max(dot(mainVectors_0.V_0, mainVectors_0.N_0), 0.0f) * _S5, 0.10000000149011612f)) * (lightColor_2 * attenuation_0) * _S5;
}


#line 355
float3 Froughness_0(float3 F0_3, float3 V_4, float3 H_3, float roughness_1)
{
    return F0_3 + (max((float3)(1.0f - roughness_1), F0_3) - F0_3) * pow(clamp(1.0f - max(dot(H_3, V_4), 0.0f), 0.0f, 1.0f), 5.0f);
}


#line 21
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
    float4 color_0 : COLOR0;
    int tex_sub_index_0 : TEXCOORD1;
    float3 normal_0 : NORMAL0;
    float3 fragmentPosition_2 : TEXCOORD2;
    float3 viewPosition_0 : TEXCOORD3;
    bool meshletVisualization_0 : TEXCOORD4;
};


#line 413
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 413
    VSOutput_0 _S6 = input_0;

    float3 _S7 = (float3)0.0f;

#line 415
    float3 albedo_2;



    if((f_matrix_0.isTex_0) > int(0))
    {

#line 419
        albedo_2 = tex_0[f_matrix_0.texIndex_0 + _S6.tex_sub_index_0].Sample(smp_0, _S6.uv_0).xyz;

#line 419
    }
    else
    {

#line 419
        albedo_2 = _S6.color_0.xyz;

#line 419
    }

#line 426
    float3 F0_4 = lerp((float3)0.03999999910593033f, albedo_2, (float3)f_material_0.metallic_0);
    float3 V_5 = normalize(_S6.viewPosition_0 - _S6.fragmentPosition_2);
    float3 N_5 = normalize(_S6.normal_0);
    MainVectors_0 _S8 = MainVectors_x24init_0(_S6.fragmentPosition_2, albedo_2, V_5, N_5, F0_4);

#line 429
    int l_0 = int(0);

#line 429
    float3 resultColor_0 = _S7;


    for(;;)
    {

#line 432
        if(l_0 < (pushConstants_0.activeDirectionalLights_0))
        {
        }
        else
        {

#line 432
            break;
        }


        float3 resultColor_1 = resultColor_0 + PBR_0(_S8, directionalLightList_0.lights_0[l_0].lightDirection_0, directionalLightList_0.lights_0[l_0].lightColor_0, false, l_0);

#line 432
        l_0 = l_0 + int(1);

#line 432
        resultColor_0 = resultColor_1;

#line 432
    }

#line 432
    l_0 = int(0);

#line 440
    for(;;)
    {

#line 440
        if(l_0 < (pushConstants_0.activePointLights_0))
        {
        }
        else
        {

#line 440
            break;
        }


        float3 resultColor_2 = resultColor_0 + PBR_0(_S8, pointLightList_0.lights_1[l_0].lightPosition_0, pointLightList_0.lights_1[l_0].lightColor_1, true, l_0);

#line 440
        l_0 = l_0 + int(1);

#line 440
        resultColor_0 = resultColor_2;

#line 440
    }

#line 449
    float3 F_1 = Froughness_0(F0_4, V_5, N_5, f_material_0.roughness_0);


    float3 _S9 = (float3)1.0f;

#line 463
    float2 brdf_0 = brdfLUT_0.Sample(iblSmp_0, float2(max(dot(N_5, V_5), 0.0f), f_material_0.roughness_0)).xy;

#line 472
    float3 resultColor_3 = (1.0f - f_material_0.metallic_0) * (_S9 - F_1) * (irradianceMap_0.Sample(iblSmp_0, N_5).xyz * albedo_2) + prefilterMap_0.SampleLevel(iblSmp_0, reflect(- V_5, N_5), f_material_0.roughness_0 * 4.0f).xyz * (F_1 * brdf_0.x + brdf_0.y) + resultColor_0;



    float3 resultColor_4 = pow(resultColor_3 / (resultColor_3 + _S9), (float3)0.45454543828964233f);

#line 476
    float4 _S10;

#line 490
    if(_S6.meshletVisualization_0)
    {

#line 490
        _S10 = float4(_S6.color_0.xyz, 1.0f);

#line 490
    }
    else
    {

#line 490
        _S10 = float4(resultColor_4, 1.0f);

#line 490
    }

#line 490
    return _S10;
}

