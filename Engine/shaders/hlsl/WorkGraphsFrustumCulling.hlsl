// dxc WorkGraphsFrustumCulling.hlsl -T lib_6_9 -E broadcastNode -Fo WorkGraphsFrustumCulling.cso
// #define MAX_VERTICES_PER_MESHLET 64
// #define MAX_PRIMITIVES_PER_MESHLET 128

struct VSInput
{
    uint position : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    int tex_sub_index : TEXCOORD1;
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

struct MeshletBounds
{
    float3 center;
    float3 extents; // bounding box half-size
};
StructuredBuffer<MeshletBounds> meshletBounds : register(t12);

struct CullingData
{
    float4 frustumPlanes[6]; // Leftm Right, Bottom, Top, Near, Far
    uint numMeshlets;
};
ConstantBuffer<CullingData> cullingData : register(b10);

ConstantBuffer<vMatrix> matrix : register(b0);

struct MeshNodeRecord
{
    uint objectIndex;
    uint meshletIndex;
};

struct CullEntryRecord
{
    uint gridSize : SV_DispatchGrid;
};

static const uint numEntryRecords = 4;

// Seems Slang Shading Language supports Work Graphs but no documentation is shown yet.
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
// Handle 32 meshlets per thread group
[NumThreads(32, 1, 1)]
// first node = broadcastNode
void CullNode(
    DispatchNodeInputRecord<CullEntryRecord> inputData,
    [MaxRecords(32)] NodeOutput<MeshNodeRecord> meshNodeOutput,
    uint dispatchThreadID : SV_DispatchThreadID
)
{
    uint meshletIndex = dispatchThreadID;
    if (meshletIndex >= cullingData.Get().numMeshlets) return;

    MeshletBounds bounds = meshletBounds[meshletIndex];
    float3 center = mul(matrix.model, float4(bounds.center, 1.0)).xyz;
    float3 extents = bounds.extents;
}

[Shader("node")]
[NodeLaunch("thread")]
void secondNode(
    ThreadNodeInputRecord<secondNodeInput> inputData,
    [MaxRecords(1)] NodeOutput<thirdNodeInput> thirdNode
)
{
    InterlockedAdd(UAV[inputData.Get().entryRecordIndex], inputData.Get().incrementValue);

    ThreadNodeOutputRecords<thirdNodeInput> outputRecords = thirdNode.GetThreadNodeOutputRecords(1);
    outputRecords.Get().entryRecordIndex = inputData.Get().entryRecordIndex;
    outputRecords.OutputComplete();
}

groupshared uint g_sum[numEntryRecords];

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(32, 1, 1)]
void thirdNode(
    [MaxRecords(32)] GroupNodeInputRecords<thirdNodeInput> inputData,
    uint threadIndex : SV_GroupIndex
)
{
    if (threadIndex >= inputData.Count()) return;

    for (uint i = 0; i < numEntryRecords; ++i)
    {
        g_sum[i] = 0;
    }

    Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

    InterlockedAdd(g_sum[inputData[threadIndex].entryRecordIndex], 1);

    Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

    if (threadIndex > 0) return;

    for (uint l = 0; l < numEntryRecords; ++l)
    {
        uint recordIndex = numEntryRecords + l;
        InterlockedAdd(UAV[recordIndex], g_sum[l]);
    }
}
