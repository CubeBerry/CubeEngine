#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 76 "slang/3D.slang"
struct Meshlet_0
{
    uint vertexCount_0;
    uint vertexOffset_0;
    uint primitiveCount_0;
    uint primitiveOffset_0;
};


StructuredBuffer<Meshlet_0 > meshlets_0 : register(t9);


#line 86
StructuredBuffer<uint > uniqueVertexIndices_0 : register(t10);


#line 10
struct VSInput_0
{
    uint position_0 : POSITION0;
    float3 normal_0 : NORMAL0;
    float2 uv_0 : TEXCOORD0;
    int tex_sub_index_0 : TEXCOORD1;
    int4 boneIDs_0;
    float4 weights_0;
};


#line 84
StructuredBuffer<VSInput_0 > uniqueVertices_0 : register(t8);


#line 33
struct vMatrix_0
{
    float4x4 model_0;
    float4x4 transposeInverseModel_0;
    float4x4 view_0;
    float4x4 projection_0;
    float4x4 decode_0;
    float4 color_0;
    float3 viewPosition_0;
};


cbuffer matrix_0 : register(b0)
{
    vMatrix_0 matrix_0;
}

#line 87
StructuredBuffer<uint > primitiveIndices_0 : register(t11);


#line 87
struct GlobalParams_0
{
    uint meshletVisualization_0;
};


#line 267
cbuffer globalParams_0 : register(b6)
{
    GlobalParams_0 globalParams_0;
}

#line 20
struct VSOutput_0
{
    float4 position_1 : SV_POSITION;
    float2 uv_1 : TEXCOORD0;
    float4 color_1 : COLOR0;
    int tex_sub_index_1 : TEXCOORD1;
    float3 normal_1 : NORMAL0;
    float3 fragmentPosition_0 : TEXCOORD2;
    float3 viewPosition_1 : TEXCOORD3;
    bool meshletVisualization_1 : TEXCOORD4;
};


#line 93
[shader("mesh")][numthreads(128, 1, 1)]
[outputtopology("triangle")]
void meshMain(uint groupThreadID_0 : SV_GroupThreadID, uint groupID_0 : SV_GroupID, out vertices VSOutput_0  verts_0[64U], out indices uint3  tris_0[128U])
{

#line 93
    uint _S1 = groupThreadID_0;

#line 93
    uint _S2 = groupID_0;

#line 100
    Meshlet_0 meshlet_0 = meshlets_0.Load(groupID_0);
    SetMeshOutputCounts(meshlet_0.vertexCount_0, meshlet_0.primitiveCount_0);

    bool meshletVisualizationEnabled_0 = (globalParams_0.meshletVisualization_0) > 0U;

    if(groupThreadID_0 < (meshlet_0.vertexCount_0))
    {

        VSInput_0 input_0 = uniqueVertices_0.Load(uniqueVertexIndices_0.Load(meshlet_0.vertexOffset_0 + _S1));

#line 114
        float3 decoded_position_0 = mul(matrix_0.decode_0, float4(float3(float((input_0.position_0) & 2047U), float(((input_0.position_0) >> int(11)) & 2047U), float(((input_0.position_0) >> int(22)) & 1023U)), 1.0f)).xyz;

        verts_0[_S1].uv_1 = input_0.uv_0;
        verts_0[_S1].meshletVisualization_1 = meshletVisualizationEnabled_0;
        if(meshletVisualizationEnabled_0)
        {


            uint hash_0 = (_S2 ^ 61U) ^ (_S2 >> int(16));
            uint hash_1 = hash_0 + (hash_0 << int(3));

            uint hash_2 = (hash_1 ^ (hash_1 >> int(4))) * 668265261U;
            uint hash_3 = hash_2 ^ (hash_2 >> int(15));

#line 132
            verts_0[_S1].color_1 = float4(float3(float(hash_3 & 255U) / 255.0f, float((hash_3 >> int(8)) & 255U) / 255.0f, float((hash_3 >> int(16)) & 255U) / 255.0f), 1.0f);

#line 118
        }
        else
        {

#line 134
            verts_0[_S1].color_1 = matrix_0.color_0;

#line 118
        }

#line 135
        verts_0[_S1].tex_sub_index_1 = input_0.tex_sub_index_0;


        verts_0[_S1].normal_1 = mul(float3x3(matrix_0.transposeInverseModel_0[int(0)].xyz, matrix_0.transposeInverseModel_0[int(1)].xyz, matrix_0.transposeInverseModel_0[int(2)].xyz), input_0.normal_0);
        float4 _S3 = float4(decoded_position_0, 1.0f);

#line 139
        verts_0[_S1].fragmentPosition_0 = mul(matrix_0.model_0, _S3).xyz;
        verts_0[_S1].viewPosition_1 = matrix_0.viewPosition_0;

        verts_0[_S1].position_1 = mul(matrix_0.projection_0, mul(matrix_0.view_0, mul(matrix_0.model_0, _S3)));

#line 105
    }

#line 145
    if(_S1 < (meshlet_0.primitiveCount_0))
    {
        uint _S4 = meshlet_0.primitiveOffset_0 + _S1 * 3U;


        tris_0[_S1] = uint3(primitiveIndices_0.Load(_S4), primitiveIndices_0.Load(_S4 + 1U), primitiveIndices_0.Load(_S4 + 2U));

#line 145
    }

#line 191
    return;
}

