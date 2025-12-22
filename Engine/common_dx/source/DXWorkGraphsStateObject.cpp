//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsStateObject.cpp
#include "DXWorkGraphsStateObject.hpp"
#include "DXHelper.hpp"
#include "DXInitializer.hpp"

// Work Graphs
DXWorkGraphsStateObject::DXWorkGraphsStateObject(
	const ComPtr<ID3D12Device14>& device,
	//const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& nodePath,
	const wchar_t* workGraphName
) : m_workGraphName(workGraphName)
{
	// 1. Create Work Graphs State Object
	std::vector<char> nodeShader = DXHelper::ReadShaderFile(nodePath);

	CD3DX12_STATE_OBJECT_DESC stateObjectDesc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

	CD3DX12_DXIL_LIBRARY_SUBOBJECT* libraryDesc = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	CD3DX12_SHADER_BYTECODE nodeShaderCode{ nodeShader.data(), nodeShader.size() };
	libraryDesc->SetDXILLibrary(&nodeShaderCode);
	DXHelper::ThrowIfFailed(device->CreateRootSignatureFromSubobjectInLibrary(0, nodeShaderCode.pShaderBytecode, nodeShaderCode.BytecodeLength, L"globalRootSignature", IID_PPV_ARGS(&m_rootSignature)));
	auto globalRootSignature = stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_rootSignature.Get());

	CD3DX12_WORK_GRAPH_SUBOBJECT* workGraphDesc = stateObjectDesc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	// Auto populate the graph
	workGraphDesc->IncludeAllAvailableNodes();
	workGraphDesc->SetProgramName(m_workGraphName);

	DXHelper::ThrowIfFailed(device->CreateStateObject(stateObjectDesc, IID_PPV_ARGS(&m_stateObject)));

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
		m_backingMemory->SetName(L"Work Graphs Backing Memory");
	}
}

#if USE_PREVIEW_SDK
// Work Graphs Mesh Nodes
DXWorkGraphsStateObject::DXWorkGraphsStateObject(
	const ComPtr<ID3D12Device14>& device,
	//const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& nodePath,
	const std::filesystem::path& pixelPath,
	const DXGI_FORMAT& depthFormat,
	const DXGI_FORMAT& renderTargetFormat,
	const wchar_t* workGraphName
) : m_workGraphName(workGraphName)
{
	// 1. Create Work Graphs State Object
	std::vector<char> nodeShader = DXHelper::ReadShaderFile(nodePath);
	std::vector<char> pixelShader = DXHelper::ReadShaderFile(pixelPath);

	CD3DX12_STATE_OBJECT_DESC stateObjectDesc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);
	// Enable graphics state for global root signature (required for Mesh Nodes)
	auto stateObjectConfig = stateObjectDesc.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
	stateObjectConfig->SetFlags(D3D12_STATE_OBJECT_FLAG_WORK_GRAPHS_USE_GRAPHICS_STATE_FOR_GLOBAL_ROOT_SIGNATURE);

	CD3DX12_SHADER_BYTECODE nodeShaderCode{ nodeShader.data(), nodeShader.size() };
	{
		CD3DX12_DXIL_LIBRARY_SUBOBJECT* libraryDesc = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		libraryDesc->SetDXILLibrary(&nodeShaderCode);
		// Export specific nodes if needed
		//libraryDesc->DefineExport(L"CullNode");
	}

	CD3DX12_SHADER_BYTECODE pixelShaderCode{ pixelShader.data(), pixelShader.size() };
	{
		CD3DX12_DXIL_LIBRARY_SUBOBJECT* libraryDesc = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		libraryDesc->SetDXILLibrary(&pixelShaderCode);
	}

	DXHelper::ThrowIfFailed(device->CreateRootSignatureFromSubobjectInLibrary(0, nodeShaderCode.pShaderBytecode, nodeShaderCode.BytecodeLength, L"globalRootSignature", IID_PPV_ARGS(&m_rootSignature)));
	auto globalRootSignature = stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_rootSignature.Get());

	CD3DX12_WORK_GRAPH_SUBOBJECT* workGraphDesc = stateObjectDesc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	// Auto populate the graph
	workGraphDesc->IncludeAllAvailableNodes();
	workGraphDesc->SetProgramName(m_workGraphName);

	// Graphics Pipeline State Subobjects for Mesh Nodes Rendering
	auto primitiveTopologySubobject = stateObjectDesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
	primitiveTopologySubobject->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	auto rasterizerSubobject = stateObjectDesc.CreateSubobject<CD3DX12_RASTERIZER_SUBOBJECT>();
	rasterizerSubobject->SetFrontCounterClockwise(true);
	rasterizerSubobject->SetFillMode(D3D12_FILL_MODE_SOLID);
	rasterizerSubobject->SetCullMode(D3D12_CULL_MODE_NONE);

	auto depthStencilSubobject = stateObjectDesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_SUBOBJECT>();
	depthStencilSubobject->SetDepthEnable(true);

	auto depthStencilFormatSubobject = stateObjectDesc.CreateSubobject<CD3DX12_DEPTH_STENCIL_FORMAT_SUBOBJECT>();
	depthStencilFormatSubobject->SetDepthStencilFormat(depthFormat);

	// @TODO Need to consider MSAA and double buffering RTVs
	auto renderTargetFormatSubobject = stateObjectDesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
	renderTargetFormatSubobject->SetNumRenderTargets(1);
	renderTargetFormatSubobject->SetRenderTargetFormat(0, renderTargetFormat);

	auto genericProgram = stateObjectDesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
	const auto genericProgramName = L"FrustumCullingGenericProgram";
	genericProgram->SetProgramName(genericProgramName);

	genericProgram->AddExport(L"MeshNode");
	genericProgram->AddExport(L"pixelMain");

	genericProgram->AddSubobject(*primitiveTopologySubobject);
	genericProgram->AddSubobject(*rasterizerSubobject);
	genericProgram->AddSubobject(*depthStencilSubobject);
	genericProgram->AddSubobject(*depthStencilFormatSubobject);
	genericProgram->AddSubobject(*renderTargetFormatSubobject);

	auto triangleNodeOverride = workGraphDesc->CreateMeshLaunchNodeOverrides(genericProgramName);
	triangleNodeOverride->NewName({ L"MeshNode", 0 });

	DXHelper::ThrowIfFailed(device->CreateStateObject(stateObjectDesc, IID_PPV_ARGS(&m_stateObject)));

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
		m_backingMemory->SetName(L"Work Graphs Backing Memory");
	}
}
#endif

D3D12_SET_PROGRAM_DESC DXWorkGraphsStateObject::GetProgramDesc() const
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

UINT DXWorkGraphsStateObject::GetEntrypointIndex(LPCWSTR nodeName)
{
	D3D12_NODE_ID nodeId = { nodeName, 0 };
	return m_workGraphProperties->GetEntrypointIndex(m_workGraphIndex, nodeId);
}
