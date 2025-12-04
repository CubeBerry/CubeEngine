//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsStateObject.cpp
#include "DXWorkGraphsStateObject.hpp"
#include "DXHelper.hpp"
#include "DXInitializer.hpp"

DXWorkGraphsStateObject::DXWorkGraphsStateObject(
	const ComPtr<ID3D12Device14>& device,
	//const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& nodePath,
	const wchar_t* workGraphName
) : m_workGraphName(workGraphName)
{
	// 1. Create Work Graphs State Object
	std::vector<char> nodeShader = DXHelper::ReadShaderFile(nodePath);

	CD3DX12_STATE_OBJECT_DESC desc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

	CD3DX12_DXIL_LIBRARY_SUBOBJECT* libraryDesc = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	CD3DX12_SHADER_BYTECODE nodeShaderCode{ nodeShader.data(), nodeShader.size() };
	libraryDesc->SetDXILLibrary(&nodeShaderCode);
	DXHelper::ThrowIfFailed(device->CreateRootSignatureFromSubobjectInLibrary(0, nodeShaderCode.pShaderBytecode, nodeShaderCode.BytecodeLength, L"globalRootSignature", IID_PPV_ARGS(&m_rootSignature)));
	auto globalRootSignature = desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_rootSignature.Get());

	CD3DX12_WORK_GRAPH_SUBOBJECT* workGraphDesc = desc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	// Auto populate the graph
	workGraphDesc->IncludeAllAvailableNodes();
	workGraphDesc->SetProgramName(m_workGraphName);

	DXHelper::ThrowIfFailed(device->CreateStateObject(desc, IID_PPV_ARGS(&m_stateObject)));

	// 2. Prepare Work Graphs
	//stateObjectProperties = m_stateObject;
	//workGraphProperties = m_stateObject;
	DXHelper::ThrowIfFailed(m_stateObject.As(&m_stateObjectProperties));
	DXHelper::ThrowIfFailed(m_stateObject.As(&m_workGraphProperties));

	m_workGraphIndex = m_workGraphProperties->GetWorkGraphIndex(m_workGraphName);
	m_workGraphProperties->GetWorkGraphMemoryRequirements(m_workGraphIndex, &m_memoryRequirements);
	if (m_memoryRequirements.MaxSizeInBytes > 0)
	{
		m_backingMemory = DXInitializer::CreateBufferResource(
			device,
			m_memoryRequirements.MaxSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_COMMON
		);
	}

	// @TODO Do I need this?
	//UINT test = m_workGraphProperties->GetNumEntrypoints(workGraphIndex);
}

D3D12_SET_PROGRAM_DESC DXWorkGraphsStateObject::GetProgramDesc()
{
	// 3. Setup Program
	D3D12_SET_PROGRAM_DESC setProgramDesc = {};
	setProgramDesc.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
	setProgramDesc.WorkGraph.ProgramIdentifier = m_stateObjectProperties->GetProgramIdentifier(m_workGraphName);
	setProgramDesc.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
	if (m_backingMemory)
	{
		setProgramDesc.WorkGraph.BackingMemory = { m_backingMemory->GetGPUVirtualAddress(), m_memoryRequirements.MaxSizeInBytes };
	}

	return setProgramDesc;
}
