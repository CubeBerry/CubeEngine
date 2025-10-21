#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 73 "3D.slang"
struct Meshlet_0
{
    uint vertexCount_0;
    uint vertexOffset_0;
    uint primitiveCount_0;
    uint primitiveOffset_0;
};


StructuredBuffer<Meshlet_0 > meshlets_0 : register(t9);


#line 83
StructuredBuffer<uint > uniqueVertexIndices_0 : register(t10);


#line 10
struct VSInput_0
{
    uint position_0 : POSITION0;
    float3 normal_0 : NORMAL0;
    float2 uv_0 : TEXCOORD0;
    int tex_sub_index_0 : TEXCOORD1;
};


#line 81
StructuredBuffer<VSInput_0 > uniqueVertices_0 : register(t8);


#line 30
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

#line 84
StructuredBuffer<uint > primitiveIndices_0 : register(t11);


#line 18
struct VSOutput_0
{
    float4 position_1 : SV_POSITION;
    float2 uv_1 : TEXCOORD0;
    float4 color_1 : COLOR0;
    int tex_sub_index_1 : TEXCOORD1;
    float3 normal_1 : NORMAL0;
    float3 fragmentPosition_0 : LIGHT0;
    float3 viewPosition_1 : LIGHT1;
};


#line 89
[shader("mesh")][numthreads(128, 1, 1)]
[outputtopology("triangle")]
void meshMain(uint groupThreadID_0 : SV_GroupThreadID, uint groupID_0 : SV_GroupID, out vertices VSOutput_0  verts_0[int(64)], out indices uint3  tris_0[int(128)])
{



    Meshlet_0 meshlet_0 = meshlets_0.Load(groupID_0);
    SetMeshOutputCounts(meshlet_0.vertexCount_0, meshlet_0.primitiveCount_0);


    if(groupThreadID_0 < meshlet_0.vertexCount_0)
    {

        VSInput_0 input_0 = uniqueVertices_0.Load(uniqueVertexIndices_0.Load(meshlet_0.vertexOffset_0 + groupThreadID_0));

#line 109
        float3 decoded_position_0 = mul(matrix_0.decode_0, float4(float3(float(input_0.position_0 & 2047U), float(input_0.position_0 >> int(11) & 2047U), float(input_0.position_0 >> int(22) & 1023U)), 1.0)).xyz;

        verts_0[groupThreadID_0].uv_1 = input_0.uv_0;
        verts_0[groupThreadID_0].color_1 = matrix_0.color_0;

#line 126
        verts_0[groupThreadID_0].tex_sub_index_1 = input_0.tex_sub_index_0;


        verts_0[groupThreadID_0].normal_1 = mul(float3x3(matrix_0.transposeInverseModel_0[int(0)].xyz, matrix_0.transposeInverseModel_0[int(1)].xyz, matrix_0.transposeInverseModel_0[int(2)].xyz), input_0.normal_0);
        float4 _S1 = float4(decoded_position_0, 1.0);

#line 130
        verts_0[groupThreadID_0].fragmentPosition_0 = mul(matrix_0.model_0, _S1).xyz;
        verts_0[groupThreadID_0].viewPosition_1 = matrix_0.viewPosition_0;

        verts_0[groupThreadID_0].position_1 = mul(matrix_0.projection_0, mul(matrix_0.view_0, mul(matrix_0.model_0, _S1)));

#line 100
    }

#line 136
    if(groupThreadID_0 < meshlet_0.primitiveCount_0)
    {
        uint _S2 = meshlet_0.primitiveOffset_0 + groupThreadID_0 * 3U;


        tris_0[groupThreadID_0] = uint3(primitiveIndices_0.Load(_S2), primitiveIndices_0.Load(_S2 + 1U), primitiveIndices_0.Load(_S2 + 2U));

#line 136
    }

#line 182
    return;
}

