// dxc WorkGraphsFrustumCulling.lib.hlsl -T lib_6_9 -E CullNode -Fo WorkGraphsFrustumCulling.lib.cso
/*
    Mesh:
    CBV - b0 ~ b1
    SRV - t0 ~ t5
    Pixel:
    CBV - b2 ~ b4
    SRV - t6 ~ t7
    Sampler - (s0, space1), (s1, space2)
    Texture - (t0, space1), (t0 ~ t2, space2)
*/
GlobalRootSignature globalRootSignature =
{
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED),"\
    "CBV(b0),"\
    "RootConstants(num32BitConstants=1, b1),"\
    "CBV(b2),"\
    "CBV(b3),"\
    "RootConstants(num32BitConstants=2, b4),"\
    "SRV(t0),"\
    "SRV(t1),"\
    "SRV(t2),"
    "SRV(t3),"\
    "SRV(t4),"\
    "SRV(t5),"\
    "SRV(t6),"\
    "SRV(t7),"\
    "StaticSampler(s0, space=1, filter=FILTER_MIN_MAG_MIP_POINT, addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP),"\
    "DescriptorTable(SRV(t0, numDescriptors = unbounded, space=1)),"\
    "StaticSampler(s1, space=2, filter=FILTER_MIN_MAG_MIP_LINEAR, addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP),"\
    "DescriptorTable(SRV(t0, numDescriptors = 3, space=2)),"\
};

#define MAX_VERTICES_PER_MESHLET 64
#define MAX_PRIMITIVES_PER_MESHLET 128

#define MAX_TEXTURES 500
#define MAX_LIGHTS 10
#define PI 3.14159265358979323846f

// ThreeDimension::StaticQuantizedVertex
struct VSInput
{
    uint position : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    int tex_sub_index : TEXCOORD1;
    uint objectId : TEXCOORD2;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    int tex_sub_index : TEXCOORD1;
    // Lighting
    float3 normal : NORMAL0;
    float3 fragmentPosition : TEXCOORD2;
    float3 viewPosition : TEXCOORD3;
    bool meshletVisualization : TEXCOORD4;
};

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
};

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
};

struct CullingData
{
    float4 frustumPlanes[6];
    uint32_t numMeshlets;
};

struct MeshNodeRecord
{
    uint objectID;
    uint meshletOffset;
    uint meshletCount;
    uint vertexIndexOffset;
    uint primitiveIndexOffset;
};

struct CullEntryRecord
{
    uint gridSize : SV_DispatchGrid;
};

StructuredBuffer<VSInput> globalUniqueVertices : register(t0);
StructuredBuffer<Meshlet> globalMeshlets : register(t1);
StructuredBuffer<vMatrix> globalVertexUniforms : register(t2);
StructuredBuffer<uint> globalUniqueVertexIndices : register(t3);
StructuredBuffer<uint> globalPrimitiveIndices : register(t4);

ConstantBuffer<CullingData> cullingData : register(b0);
uniform uint meshletVisualization : register(b1);

struct MeshletBounds
{
    float3 center;
    float3 extents; // bounding box half-size
};
StructuredBuffer<MeshletBounds> globalMeshletBounds : register(t5);

bool IsVisible(float3 center, float3 extents)
{
    for (int i = 0; i < 6; ++i)
    {
        float4 plane = cullingData.frustumPlanes[i];
        float d = dot(center, plane.xyz) + plane.w;
        float r = dot(extents, abs(plane.xyz));
        if (d + r < 0.0f) return false;
    }
    return true;
}

// Seems Slang Shading Language supports Work Graphs but no documentation is shown yet.
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
// Handle 32 meshlets per thread group
[NumThreads(32, 1, 1)]
// first node = broadcastNode
void CullNode(
    uint dispatchThreadID: SV_DispatchThreadID,
    DispatchNodeInputRecord<CullEntryRecord> inputRecord,
    [MaxRecords(32)] NodeOutput<MeshNodeRecord> meshNodeOutput
)
{
    uint globalMeshletIndex = dispatchThreadID;
    if (globalMeshletIndex >= cullingData.numMeshlets) return;

    MeshletBounds bounds = globalMeshletBounds[globalMeshletIndex];
    uint objectID = 0; // @TODO get object ID from a buffer
    vMatrix vertexUniform = globalVertexUniforms[objectID];

    float3 worldCenter = mul(vertexUniform.model, float4(bounds.center, 1.0)).xyz;
    // float3 worldExtents = mul(vertexUniform.model, float4(bounds.extents, 0.0)).xyz;
    float3 worldExtents = bounds.extents;

    if (IsVisible(worldCenter, worldExtents))
    {
        ThreadNodeOutputRecords<MeshNodeRecord> outRecord = meshNodeOutput.GetThreadNodeOutputRecords(1);

        outRecord.Get().objectID = objectID;
        outRecord.Get().meshletOffset = globalMeshletIndex;
        outRecord.Get().meshletCount = 1;
        // @TODO Remove offsets later
        outRecord.Get().vertexIndexOffset = 0;
        outRecord.Get().primitiveIndexOffset = 0;

        outRecord.OutputComplete();
    }
}

[Shader("node")]
[NodeLaunch("mesh")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MeshNode(
    DispatchNodeInputRecord<MeshNodeRecord> inputRecord,
    uint groupThreadID: SV_GroupThreadID,
    uint dispatchThreadID: SV_DispatchThreadID,
    out vertices VSOutput verts[MAX_VERTICES_PER_MESHLET],
    out indices uint3 tris[MAX_PRIMITIVES_PER_MESHLET]
)
{
    MeshNodeRecord record = inputRecord.Get();

    Meshlet meshlet = globalMeshlets[record.meshletOffset];
    vMatrix matrix = globalVertexUniforms[record.objectID];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    // bool meshletVisualizationEnabled = meshletVisualization > 0;
    // MAX_VERTICES_PER_MESHLET == 64, MAX_PRIMITIVES_PER_MESHLET == 128
    if (groupThreadID < meshlet.vertexCount)
    {
        uint vertexIndex = globalUniqueVertexIndices[meshlet.vertexOffset + groupThreadID];
        VSInput input = globalUniqueVertices[vertexIndex];

        // 11, 11, 10
        float x = float(input.position & 0x7FFu);
        float y = float((input.position >> 11) & 0x7FFu);
        float z = float((input.position >> 22) & 0x3FFu);
        float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

        verts[groupThreadID].uv = input.uv;
        // verts[groupThreadID].meshletVisualization = meshletVisualizationEnabled;
        // if (meshletVisualizationEnabled)
        // {
        //     // Mesh Shader Debug Color
        //     uint hash = groupID.x;
        //     hash = (hash ^ 61) ^ (hash >> 16);
        //     hash = hash + (hash << 3);
        //     hash = hash ^ (hash >> 4);
        //     hash = hash * 0x27d4eb2d;
        //     hash = hash ^ (hash >> 15);
        //     float3 debugColor = float3(
        //         float(hash & 0xFF) / 255.0f,
        //         float((hash >> 8) & 0xFF) / 255.0f,
        //         float((hash >> 16) & 0xFF) / 255.0f
        //     );
        //     verts[groupThreadID].color = float4(debugColor, 1.0f);
        // }
        // else verts[groupThreadID].color = matrix.color;
        verts[groupThreadID].tex_sub_index = input.tex_sub_index;

        // Lighting
        verts[groupThreadID].normal = mul((float3x3)matrix.transposeInverseModel, input.normal);
        verts[groupThreadID].fragmentPosition = mul(matrix.model, float4(decoded_position, 1.0f)).xyz;
        verts[groupThreadID].viewPosition = matrix.viewPosition;

        verts[groupThreadID].position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(decoded_position, 1.0))));
    }

    if (groupThreadID < meshlet.primitiveCount)
    {
        uint i0 = globalPrimitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 0];
        uint i1 = globalPrimitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 1];
        uint i2 = globalPrimitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 2];
        tris[groupThreadID] = uint3(i0, i1, i2);
    }
}
