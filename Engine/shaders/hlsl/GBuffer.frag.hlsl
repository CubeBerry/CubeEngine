#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 208 "slang/GBuffer.slang"
struct fMatrix_0
{
    int isTex_0;
    int texIndex_0;
};


#line 213
cbuffer f_matrix_0 : register(b1)
{
    fMatrix_0 f_matrix_0;
}
Texture2D<float4 >  tex_0[int(500)] : register(t0, space1);


#line 216
SamplerState smp_0 : register(s0, space1);


#line 222
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

#line 200
struct PSOutput_0
{
    float4 Albedo_0 : SV_Target0;
    float4 Normal_0 : SV_Target1;
    float4 Position_0 : SV_Target2;
    float4 Material_0 : SV_Target3;
};


#line 20
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    float2 uv_0 : TEXCOORD0;
    float4 color_0 : COLOR0;
    int tex_sub_index_0 : TEXCOORD1;
    int4 boneIDs_0 : BLENDINDICES0;
    float4 weights_0 : BLENDWEIGHTS0;
    float3 normal_0 : NORMAL0;
    float3 fragmentPosition_0 : TEXCOORD2;
    float3 viewPosition_0 : TEXCOORD3;
    bool meshletVisualization_0 : TEXCOORD4;
};


#line 235
PSOutput_0 fragmentMain(VSOutput_0 input_0)
{

#line 235
    VSOutput_0 _S1 = input_0;

    PSOutput_0 output_0;

#line 237
    float4 albedo_0;


    if(!input_0.meshletVisualization_0)
    {

        if((f_matrix_0.isTex_0) > int(0))
        {

#line 243
            albedo_0 = tex_0[f_matrix_0.texIndex_0 + _S1.tex_sub_index_0].Sample(smp_0, _S1.uv_0);

#line 243
        }
        else
        {

#line 243
            albedo_0 = _S1.color_0;

#line 243
        }

#line 240
    }
    else
    {

#line 240
        albedo_0 = float4(_S1.color_0.xyz, 1.0f);

#line 240
    }

#line 251
    if((albedo_0.w) < 0.10000000149011612f)
    {

#line 251
        discard;

#line 251
    }

    output_0.Albedo_0 = albedo_0;
    output_0.Normal_0 = float4(normalize(_S1.normal_0), 1.0f);
    output_0.Position_0 = float4(_S1.fragmentPosition_0, 1.0f);

    output_0.Material_0 = float4(f_material_0.metallic_0, f_material_0.roughness_0, 1.0f, 0.0f);

    return output_0;
}

