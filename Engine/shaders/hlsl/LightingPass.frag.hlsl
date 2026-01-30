#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 73 "slang/LightingPass.slang"
Texture2D<float4 > gAlbedo_0 : register(t0, space1);



SamplerState gSampler_0 : register(s0, space1);


#line 74
Texture2D<float4 > gNormal_0 : register(t1, space1);


#line 75
Texture2D<float4 > gPosition_0 : register(t2, space1);


#line 76
Texture2D<float4 > gMaterial_0 : register(t3, space1);


#line 61
struct PushConstants_0
{
    float3 viewPosition_0;
    int activeDirectionalLights_0;
    int activePointLights_0;
};

cbuffer pushConstants_0 : register(b2)
{
    PushConstants_0 pushConstants_0;
}

#line 29
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


#line 41
cbuffer directionalLightList_0 : register(b0)
{
    fDirectionalLightList_0 directionalLightList_0;
}

#line 43
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


#line 59
cbuffer pointLightList_0 : register(b1)
{
    fPointLightList_0 pointLightList_0;
}

#line 80
TextureCube<float4 > irradianceMap_0 : register(t0, space2);


SamplerState iblSmp_0 : register(s1, space2);


#line 81
TextureCube<float4 > prefilterMap_0 : register(t1, space2);


#line 82
Texture2D<float4 > brdfLUT_0 : register(t2, space2);


#line 175
struct MainVectors_0
{
    float3 worldPosition_0;
    float3 albedo_0;
    float4 material_0;
    float3 V_0;
    float3 N_0;
    float3 F0_0;
};


#line 175
MainVectors_0 MainVectors_x24init_0(float3 worldPosition_1, float3 albedo_1, float4 material_1, float3 V_1, float3 N_1, float3 F0_1)
{

#line 175
    MainVectors_0 _S1;

    _S1.worldPosition_0 = worldPosition_1;
    _S1.albedo_0 = albedo_1;
    _S1.material_0 = material_1;
    _S1.V_0 = V_1;
    _S1.N_0 = N_1;
    _S1.F0_0 = F0_1;

#line 175
    return _S1;
}


#line 165
float3 F_0(float3 F0_2, float3 V_2, float3 H_0)
{
    return F0_2 + ((float3)1.0f - F0_2) * pow(1.0f - max(dot(V_2, H_0), 0.0f), 5.0f);
}


#line 135
float D_0(float alpha_0, float3 N_2, float3 H_1)
{
    float numerator_0 = pow(alpha_0, 2.0f);

#line 143
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


#line 186
float3 PBR_0(MainVectors_0 mainVectors_0, float3 lightPosition_1, float3 lightColor_2, bool isPointLight_0, int lightIndex_0)
{
    float metallic_0 = mainVectors_0.material_0.x;
    float roughness_0 = mainVectors_0.material_0.y;

#line 189
    float distance_0;


    if(isPointLight_0)
    {

#line 192
        distance_0 = length(lightPosition_1 - mainVectors_0.worldPosition_0);

#line 192
    }
    else
    {

#line 192
        distance_0 = 1.0f;

#line 192
    }

#line 192
    float attenuation_0;

#line 192
    float3 L_1;



    if(isPointLight_0)
    {

        float _S2 = 1.0f / (pointLightList_0.lights_1[lightIndex_0].constant_0 + pointLightList_0.lights_1[lightIndex_0].linear_0 * distance_0 + pointLightList_0.lights_1[lightIndex_0].quadratic_0 * (distance_0 * distance_0));

#line 199
        L_1 = normalize(lightPosition_1 - mainVectors_0.worldPosition_0);

#line 199
        attenuation_0 = _S2;

#line 196
    }
    else
    {

#line 196
        L_1 = normalize(- lightPosition_1);

#line 196
        attenuation_0 = 1.0f;

#line 196
    }

#line 208
    float3 H_2 = normalize(mainVectors_0.V_0 + L_1);


    float3 Ks_0 = F_0(mainVectors_0.F0_0, mainVectors_0.V_0, H_2);

#line 217
    float alpha_3 = roughness_0 * roughness_0;

    float _S3 = max(dot(L_1, mainVectors_0.N_0), 0.0f);

#line 226
    return ((1.0f - metallic_0) * ((float3)1.0f - Ks_0) * (mainVectors_0.albedo_0 / 3.14159274101257324f) + D_0(alpha_3, mainVectors_0.N_0, H_2) * G_0(alpha_3, mainVectors_0.N_0, mainVectors_0.V_0, L_1) * Ks_0 / max(4.0f * max(dot(mainVectors_0.V_0, mainVectors_0.N_0), 0.0f) * _S3, 0.10000000149011612f)) * (lightColor_2 * attenuation_0) * _S3;
}


#line 170
float3 Froughness_0(float3 F0_3, float3 V_4, float3 H_3, float roughness_1)
{
    return F0_3 + (max((float3)(1.0f - roughness_1), F0_3) - F0_3) * pow(clamp(1.0f - max(dot(H_3, V_4), 0.0f), 0.0f, 1.0f), 5.0f);
}


#line 10
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
};


#line 230
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{

#line 230
    VSOutput_0 _S4 = input_0;

    float3 _S5 = (float3)0.0f;

    float4 albedoSample_0 = gAlbedo_0.Sample(gSampler_0, input_0.uv_0);
    if((albedoSample_0.w) < 0.00999999977648258f)
    {

#line 235
        discard;

#line 235
    }
    float3 albedo_2 = albedoSample_0.xyz;

    float3 worldPosition_2 = gPosition_0.Sample(gSampler_0, _S4.uv_0).xyz;
    float4 material_2 = gMaterial_0.Sample(gSampler_0, _S4.uv_0);

    float metallic_1 = material_2.x;
    float roughness_2 = material_2.y;



    float3 F0_4 = lerp((float3)0.03999999910593033f, albedo_2, (float3)metallic_1);
    float3 V_5 = normalize(pushConstants_0.viewPosition_0 - worldPosition_2);
    float3 N_5 = normalize(gNormal_0.Sample(gSampler_0, _S4.uv_0).xyz);
    MainVectors_0 _S6 = MainVectors_x24init_0(worldPosition_2, albedo_2, material_2, V_5, N_5, F0_4);

#line 249
    int l_0 = int(0);

#line 249
    float3 resultColor_0 = _S5;


    for(;;)
    {

#line 252
        if(l_0 < (pushConstants_0.activeDirectionalLights_0))
        {
        }
        else
        {

#line 252
            break;
        }


        float3 resultColor_1 = resultColor_0 + PBR_0(_S6, directionalLightList_0.lights_0[l_0].lightDirection_0, directionalLightList_0.lights_0[l_0].lightColor_0, false, l_0);

#line 252
        l_0 = l_0 + int(1);

#line 252
        resultColor_0 = resultColor_1;

#line 252
    }

#line 252
    l_0 = int(0);

#line 260
    for(;;)
    {

#line 260
        if(l_0 < (pushConstants_0.activePointLights_0))
        {
        }
        else
        {

#line 260
            break;
        }


        float3 resultColor_2 = resultColor_0 + PBR_0(_S6, pointLightList_0.lights_1[l_0].lightPosition_0, pointLightList_0.lights_1[l_0].lightColor_1, true, l_0);

#line 260
        l_0 = l_0 + int(1);

#line 260
        resultColor_0 = resultColor_2;

#line 260
    }

#line 269
    float3 F_1 = Froughness_0(F0_4, V_5, N_5, roughness_2);


    float3 _S7 = (float3)1.0f;

#line 283
    float2 brdf_0 = brdfLUT_0.Sample(iblSmp_0, float2(max(dot(N_5, V_5), 0.0f), roughness_2)).xy;

#line 292
    float3 resultColor_3 = (1.0f - metallic_1) * (_S7 - F_1) * (irradianceMap_0.Sample(iblSmp_0, N_5).xyz * albedo_2) + prefilterMap_0.SampleLevel(iblSmp_0, reflect(- V_5, N_5), roughness_2 * 4.0f).xyz * (F_1 * brdf_0.x + brdf_0.y) + resultColor_0;

#line 309
    return float4(pow(resultColor_3 / (resultColor_3 + _S7), (float3)0.45454543828964233f), 1.0f);
}

