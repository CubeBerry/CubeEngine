#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 20 "slang/LocalLightingPass.slang"
struct PushConstants_0
{
    float4x4 model_0;
    float4x4 viewProjection_0;
    float3 viewPosition_0;
    int lightIndex_0;
    float2 screenSize_0;
};

cbuffer pushConstants_0 : register(b1)
{
    PushConstants_0 pushConstants_0;
}

#line 64
Texture2D<float4 > gAlbedo_0 : register(t0, space1);



SamplerState gSampler_0 : register(s0, space1);


#line 65
Texture2D<float4 > gNormal_0 : register(t1, space1);


#line 66
Texture2D<float4 > gPosition_0 : register(t2, space1);


#line 67
Texture2D<float4 > gMaterial_0 : register(t3, space1);


#line 45
struct fPointLight_0
{
    float3 lightPosition_0;
    float ambientStrength_0;
    float3 lightColor_0;
    float specularStrength_0;
    float constant_0;
    float linear_0;
    float quadratic_0;
    float radius_0;
};


struct fPointLightList_0
{
    fPointLight_0  lights_0[int(10)];
};


#line 62
cbuffer pointLightList_0 : register(b0)
{
    fPointLightList_0 pointLightList_0;
}

#line 166
struct MainVectors_0
{
    float3 worldPosition_0;
    float3 albedo_0;
    float4 material_0;
    float3 V_0;
    float3 N_0;
    float3 F0_0;
};


#line 166
MainVectors_0 MainVectors_x24init_0(float3 worldPosition_1, float3 albedo_1, float4 material_1, float3 V_1, float3 N_1, float3 F0_1)
{

#line 166
    MainVectors_0 _S1;

    _S1.worldPosition_0 = worldPosition_1;
    _S1.albedo_0 = albedo_1;
    _S1.material_0 = material_1;
    _S1.V_0 = V_1;
    _S1.N_0 = N_1;
    _S1.F0_0 = F0_1;

#line 166
    return _S1;
}


#line 156
float3 F_0(float3 F0_2, float3 V_2, float3 H_0)
{
    return F0_2 + ((float3)1.0f - F0_2) * pow(1.0f - max(dot(V_2, H_0), 0.0f), 5.0f);
}


#line 126
float D_0(float alpha_0, float3 N_2, float3 H_1)
{
    float numerator_0 = pow(alpha_0, 2.0f);

#line 134
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


#line 177
float3 PBR_0(MainVectors_0 mainVectors_0, float3 lightPosition_1, float3 lightColor_1, int lightIndex_1)
{

    float roughness_0 = mainVectors_0.material_0.y;

#line 185
    float3 L_1 = normalize(lightPosition_1 - mainVectors_0.worldPosition_0);


    float3 H_2 = normalize(mainVectors_0.V_0 + L_1);


    float3 Ks_0 = F_0(mainVectors_0.F0_0, mainVectors_0.V_0, H_2);

#line 197
    float alpha_3 = roughness_0 * roughness_0;

    float _S2 = max(dot(L_1, mainVectors_0.N_0), 0.0f);

#line 206
    return ((1.0f - mainVectors_0.material_0.x) * ((float3)1.0f - Ks_0) * (mainVectors_0.albedo_0 / 3.14159274101257324f) + D_0(alpha_3, mainVectors_0.N_0, H_2) * G_0(alpha_3, mainVectors_0.N_0, mainVectors_0.V_0, L_1) * Ks_0 / max(4.0f * max(dot(mainVectors_0.V_0, mainVectors_0.N_0), 0.0f) * _S2, 0.10000000149011612f)) * lightColor_1 * _S2;
}


#line 15
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
};


#line 210
float4 fragmentMain(VSOutput_0 input_0) : SV_TARGET
{
    float2 uv_0 = input_0.position_0.xy / pushConstants_0.screenSize_0;

    float4 albedoSample_0 = gAlbedo_0.Sample(gSampler_0, uv_0);
    if((albedoSample_0.w) < 0.00999999977648258f)
    {

#line 215
        discard;

#line 215
    }
    float3 albedo_2 = albedoSample_0.xyz;

    float3 worldPosition_2 = gPosition_0.Sample(gSampler_0, uv_0).xyz;
    float4 material_2 = gMaterial_0.Sample(gSampler_0, uv_0);

#line 229
    MainVectors_0 mainVectors_1 = MainVectors_x24init_0(worldPosition_2, albedo_2, material_2, normalize(pushConstants_0.viewPosition_0 - worldPosition_2), normalize(gNormal_0.Sample(gSampler_0, uv_0).xyz), lerp((float3)0.03999999910593033f, albedo_2, (float3)material_2.x));


    fPointLight_0 light_0 = pointLightList_0.lights_0[pushConstants_0.lightIndex_0];

    float distance_0 = length(pointLightList_0.lights_0[pushConstants_0.lightIndex_0].lightPosition_0 - worldPosition_2);
    if(distance_0 > (pointLightList_0.lights_0[pushConstants_0.lightIndex_0].radius_0))
    {

#line 235
        discard;

#line 235
    }

#line 254
    return float4(PBR_0(mainVectors_1, light_0.lightPosition_0, light_0.lightColor_0, pushConstants_0.lightIndex_0) * (1.0f / (distance_0 * distance_0)), 1.0f);
}

