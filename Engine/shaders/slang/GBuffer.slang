#extension GL_EXT_nonuniform_qualifier : enable

// #pragma pack_matrix(row_major)

#define MAX_TEXTURES 500
#define MAX_LIGHTS 200
#define PI 3.14159265358979323846f

// Vertex Shader
struct VSInput
{
    uint position : POSITION0;
    float3 normal : NORMAL0;
    int tex_sub_index : TEXCOORD1;
    float2 uv : TEXCOORD0;
    int4 boneIDs : BLENDINDICES0;
    float4 weights : BLENDWEIGHTS0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    int tex_sub_index : TEXCOORD1;
    int4 boneIDs : BLENDINDICES0;
    float4 weights : BLENDWEIGHTS0;
    // G-Buffer
    float3 normal : NORMAL0; // Normal
    float3 fragmentPosition : TEXCOORD2; // World Position
    float3 viewPosition : TEXCOORD3;
    bool meshletVisualization : TEXCOORD4;
}

struct vMatrix
{
    float4x4 model;
    // @TODO move to push constants
    float4x4 transposeInverseModel;
    float4x4 view;
    float4x4 projection;
    float4x4 decode;
    float4 color;
    // @TODO move to push constants
    float3 viewPosition;
}
[[vk::binding(0, 0)]] ConstantBuffer<vMatrix> matrix : register(b0, space0);

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // 11, 11, 10
    float x = float(input.position & 0x7FFu);
    float y = float((input.position >> 11) & 0x7FFu);
    float z = float((input.position >> 22) & 0x3FFu);
    float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

    output.uv = input.uv;
    output.color = matrix.color;
    output.tex_sub_index = input.tex_sub_index;

    output.boneIDs = input.boneIDs;
    output.weights = input.weights;
    output.meshletVisualization = false;

    // Lighting
    output.normal = mul((float3x3)matrix.transposeInverseModel, input.normal);
    output.fragmentPosition = mul(matrix.model, float4(decoded_position, 1.0f)).xyz;
    output.viewPosition = matrix.viewPosition;

    output.position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(decoded_position, 1.0))));
    
    return output;
}

// Mesh Shader
#define MAX_VERTICES_PER_MESHLET 64
#define MAX_PRIMITIVES_PER_MESHLET 128

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
};

StructuredBuffer<VSInput> uniqueVertices : register(t8);
StructuredBuffer<Meshlet> meshlets : register(t9);
StructuredBuffer<uint> uniqueVertexIndices : register(t10);
StructuredBuffer<uint> primitiveIndices : register(t11);
uniform uint meshletVisualization : register(b6, space0);

[numthreads(128, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
    uint groupThreadID: SV_GroupThreadID,
    uint groupID: SV_GroupID,
    OutputVertices<VSOutput, MAX_VERTICES_PER_MESHLET> verts,
    OutputIndices<uint3, MAX_PRIMITIVES_PER_MESHLET> tris
)
{
    Meshlet meshlet = meshlets[groupID.x];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    bool meshletVisualizationEnabled = meshletVisualization > 0;
    // MAX_VERTICES_PER_MESHLET == 64, MAX_PRIMITIVES_PER_MESHLET == 128
    if (groupThreadID < meshlet.vertexCount)
    {
        uint vertexIndex = uniqueVertexIndices[meshlet.vertexOffset + groupThreadID];
        VSInput input = uniqueVertices[vertexIndex];

        // 11, 11, 10
        float x = float(input.position & 0x7FFu);
        float y = float((input.position >> 11) & 0x7FFu);
        float z = float((input.position >> 22) & 0x3FFu);
        float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

        verts[groupThreadID].uv = input.uv;
        verts[groupThreadID].meshletVisualization = meshletVisualizationEnabled;
        if (meshletVisualizationEnabled)
        {
            // Mesh Shader Debug Color
            uint hash = groupID.x;
            hash = (hash ^ 61) ^ (hash >> 16);
            hash = hash + (hash << 3);
            hash = hash ^ (hash >> 4);
            hash = hash * 0x27d4eb2d;
            hash = hash ^ (hash >> 15);
            float3 debugColor = float3(
                float(hash & 0xFF) / 255.0f,
                float((hash >> 8) & 0xFF) / 255.0f,
                float((hash >> 16) & 0xFF) / 255.0f
            );
            verts[groupThreadID].color = float4(debugColor, 1.0f);
        }
        else verts[groupThreadID].color = matrix.color;
        verts[groupThreadID].tex_sub_index = input.tex_sub_index;

        // Lighting
        verts[groupThreadID].normal = mul((float3x3)matrix.transposeInverseModel, input.normal);
        verts[groupThreadID].fragmentPosition = mul(matrix.model, float4(decoded_position, 1.0f)).xyz;
        verts[groupThreadID].viewPosition = matrix.viewPosition;

        verts[groupThreadID].position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(decoded_position, 1.0))));
    }

    if (groupThreadID < meshlet.primitiveCount)
    {
        uint i0 = primitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 0];
        uint i1 = primitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 1];
        uint i2 = primitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 2];
        tris[groupThreadID] = uint3(i0, i1, i2);
    }

    // https://gpuopen.com/learn/mesh_shaders/mesh_shaders-optimization_and_best_practices/
    // It differs based on GPU, max 128 vertices and 256 primitives are recommended to Radeon
    // MAX_VERTICES_PER_MESHLET == 128, MAX_PRIMITIVES_PER_MESHLET == 256
    // for (uint i = 0; i < 2; ++i)
    // {
    //     const uint primitiveId = groupThreadID + i * 128;

    //     if (primitiveId < meshlet.vertexCount)
    //     {
    //         uint vertexIndex = uniqueVertexIndices[meshlet.vertexOffset + primitiveId];
    //         VSInput input = uniqueVertices[vertexIndex];

    //         // 11, 11, 10
    //         float x = float(input.position & 0x7FFu);
    //         float y = float((input.position >> 11) & 0x7FFu);
    //         float z = float((input.position >> 22) & 0x3FFu);
    //         float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

    //         verts[primitiveId].uv = input.uv;
    //         verts[primitiveId].color = matrix.color;
    //         verts[primitiveId].tex_sub_index = input.tex_sub_index;

    //         // Lighting
    //         verts[primitiveId].normal = mul((float3x3)matrix.transposeInverseModel, input.normal);
    //         verts[primitiveId].fragmentPosition = mul(matrix.model, float4(decoded_position, 1.0f)).xyz;
    //         verts[primitiveId].viewPosition = matrix.viewPosition;

    //         verts[primitiveId].position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(decoded_position, 1.0))));
    //     }

    //     if (primitiveId < meshlet.primitiveCount)
    //     {
    //         uint i0 = primitiveIndices[meshlet.primitiveOffset + primitiveId * 3 + 0];
    //         uint i1 = primitiveIndices[meshlet.primitiveOffset + primitiveId * 3 + 1];
    //         uint i2 = primitiveIndices[meshlet.primitiveOffset + primitiveId * 3 + 2];
    //         tris[primitiveId] = uint3(i0, i1, i2);
    //     }
    // }
}

// Fragment Shader
struct PSOutput
{
    float4 Albedo : SV_Target0; // DXGI_FORMAT_R8G8B8A8_UNORM
    float4 Normal : SV_Target1; // DXGI_FORMAT_R16G16B16A16_FLOAT
    float4 Position : SV_Target2; // DXGI_FORMAT_R16G16B16A16_FLOAT
    float4 Material : SV_Target3; // DXGI_FORMAT_R8G8B8A8_UNORM
}

struct fMatrix
{
    int isTex;
    int texIndex;
}
[[vk::binding(0, 1)]] ConstantBuffer<fMatrix> f_matrix : register(b1, space0);

#if defined(__hlsl__)
SamplerState smp : register(s0, space1);
Texture2D tex[MAX_TEXTURES] : register(t0, space1);
#else
[[vk::binding(1, 1)]] Sampler2D tex[MAX_TEXTURES];
#endif

struct fMaterial
{
    // Blinn-Phong lighting
    float3 specularColor;
    float shininess;

    // PBR
    float metallic;
    float roughness;
}
[[vk::binding(2, 1)]] ConstantBuffer<fMaterial> f_material : register(b2, space0);

[shader("fragment")]
PSOutput fragmentMain(VSOutput input)
{
    PSOutput output;

    float4 albedo = float4(0.0);
    if (!input.meshletVisualization)
    {
#if defined(__hlsl__)
        if (f_matrix.isTex > 0) albedo = tex[f_matrix.texIndex + input.tex_sub_index].Sample(smp, input.uv);
#else
        if (f_matrix.isTex > 0) albedo = tex[f_matrix.texIndex + input.tex_sub_index].Sample(input.uv);
#endif
        else albedo = input.color;
    }
    else albedo = float4(input.color.rgb, 1.0);

    if (albedo.a < 0.1) discard;

    output.Albedo = albedo;
    output.Normal = float4(normalize(input.normal), 1.0);
    output.Position = float4(input.fragmentPosition, 1.0);
    // R == Metallic, G == Roughness, B == AO, A == (Unused)
    output.Material = float4(f_material.metallic, f_material.roughness, 1.0, 0.0);

    return output;
}
