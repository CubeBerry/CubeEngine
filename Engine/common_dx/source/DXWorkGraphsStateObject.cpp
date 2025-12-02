//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsStateObject.cpp
#include "DXWorkGraphsStateObject.hpp"
#include "DXHelper.hpp"

DXWorkGraphsStateObject::DXWorkGraphsStateObject(
	const ComPtr<ID3D12Device14>& device,
	//const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& nodePath
	)
{
	ComPtr<ID3DBlob> errorMessages;

	std::vector<char> nodeShader = DXHelper::ReadShaderFile(nodePath);

	CD3DX12_STATE_OBJECT_DESC desc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

	CD3DX12_DXIL_LIBRARY_SUBOBJECT* libraryDesc = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	CD3DX12_SHADER_BYTECODE nodeShaderCode{ nodeShader.data(), nodeShader.size() };
	libraryDesc->SetDXILLibrary(&nodeShaderCode);
	DXHelper::ThrowIfFailed(device->CreateRootSignatureFromSubobjectInLibrary(0, nodeShaderCode.pShaderBytecode, nodeShaderCode.BytecodeLength, L"Global Root Signature", IID_PPV_ARGS(&m_rootSignature)));

	CD3DX12_WORK_GRAPH_SUBOBJECT* workGraphDesc = desc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	// Auto populate the graph
	workGraphDesc->IncludeAllAvailableNodes();
	LPCWSTR workGraphName = L"WorkGraphsTutorial";
	workGraphDesc->SetProgramName(workGraphName);

	DXHelper::ThrowIfFailed(device->CreateStateObject(desc, IID_PPV_ARGS(&m_stateObject)));
}
