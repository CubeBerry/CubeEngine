//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsStateObject.cpp
#include "DXWorkGraphsStateObject.hpp"
#include "DXHelper.hpp"
#include "DXInitializer.hpp"

DXWorkGraphsStateObject::DXWorkGraphsStateObject(
	const ComPtr<ID3D12Device14>& device,
	//const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& nodePath
	)
{
	// 1. Create Work Graphs State Object
	std::vector<char> nodeShader = DXHelper::ReadShaderFile(nodePath);

	CD3DX12_STATE_OBJECT_DESC desc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

	CD3DX12_DXIL_LIBRARY_SUBOBJECT* libraryDesc = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	CD3DX12_SHADER_BYTECODE nodeShaderCode{ nodeShader.data(), nodeShader.size() };
	libraryDesc->SetDXILLibrary(&nodeShaderCode);
	DXHelper::ThrowIfFailed(device->CreateRootSignatureFromSubobjectInLibrary(0, nodeShaderCode.pShaderBytecode, nodeShaderCode.BytecodeLength, L"globalRootSignature", IID_PPV_ARGS(&m_rootSignature)));

	CD3DX12_WORK_GRAPH_SUBOBJECT* workGraphDesc = desc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	// Auto populate the graph
	workGraphDesc->IncludeAllAvailableNodes();
	LPCWSTR workGraphName = L"WorkGraphsTutorial";
	workGraphDesc->SetProgramName(workGraphName);

	DXHelper::ThrowIfFailed(device->CreateStateObject(desc, IID_PPV_ARGS(&m_stateObject)));

	// 2. Prepare Work Graphs
	ComPtr<ID3D12StateObjectProperties1> stateObjectProperties;
	ComPtr<ID3D12WorkGraphProperties> workGraphProperties;
	//stateObjectProperties = m_stateObject;
	//workGraphProperties = m_stateObject;
	DXHelper::ThrowIfFailed(m_stateObject.As(&stateObjectProperties));
	DXHelper::ThrowIfFailed(m_stateObject.As(&workGraphProperties));

	UINT workGraphIndex = workGraphProperties->GetWorkGraphIndex(workGraphName);
	D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS memoryRequirements = {};
	workGraphProperties->GetWorkGraphMemoryRequirements(workGraphIndex, &memoryRequirements);
	if (memoryRequirements.MaxSizeInBytes > 0)
	{
		m_backingMemory = DXInitializer::CreateBufferResource(
			device,
			memoryRequirements.MaxSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_COMMON
		);
	}

	//UINT test = workGraphProperties->GetNumEntrypoints(workGraphIndex);

	// 3. Setup Program
	D3D12_SET_PROGRAM_DESC setProgramDesc = {};
	setProgramDesc.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
	setProgramDesc.WorkGraph.ProgramIdentifier = stateObjectProperties->GetProgramIdentifier(workGraphName);
	setProgramDesc.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
	if (m_backingMemory)
	{
		setProgramDesc.WorkGraph.BackingMemory = { m_backingMemory->GetGPUVirtualAddress(), memoryRequirements.MaxSizeInBytes };
	}
}
