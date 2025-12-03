// dxc WorkGraphs.hlsl -T lib_6_8 -E broadcastNode -Fo WorkGraphs.cso
GlobalRootSignature globalRootSignature =
{
    "DescriptorTable(UAV(u0))"
};
RWStructuredBuffer<uint> UAV : register(u0);

struct entryRecord
{
    uint gridSize : SV_DispatchGrid;
    uint recordIndex;
};

struct secondNodeInput
{
    uint entryRecordIndex;
    uint incrementValue;
};

struct thirdNodeInput
{
    uint entryRecordIndex;
};

static const uint numEntryRecords = 4;

// Seems Slang Shading Language supports Work Graphs but no documentation is shown yet.
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]

[NumThreads(2, 1, 1)]
// first node = broadcastNode
void broadcastNode(
    DispatchNodeInputRecord<entryRecord> inputData,
    [MaxRecords(2)] NodeOutput<secondNodeInput> secondNode,
    uint threadIndex : SV_GroupIndex,
    uint dispatchThreadID : SV_DispatchThreadID
)
{
    GroupNodeOutputRecords<secondNodeInput> outputRecords = secondNode.GetGroupNodeOutputRecords(2);

    outputRecords[threadIndex].entryRecordIndex = inputData.Get().recordIndex;

    outputRecords[threadIndex].incrementValue = dispatchThreadID * 2 + threadIndex + 1;
    outputRecords.OutputComplete();
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
