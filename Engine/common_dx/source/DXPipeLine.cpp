//Author: JEYOON YU
//Project: CubeEngine
//File: DXPipeLine.cpp
#include "DXPipeLine.hpp"

#include <d3dcompiler.h>

DXPipeLine::DXPipeLine(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& vertexPath, const std::filesystem::path& pixelPath,
	std::initializer_list<DXAttributeLayout> layout,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology
	)
{
#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	HRESULT hr;

	// Compile vertex shader
	hr = D3DCompileFromFile(
		vertexPath.c_str(),
		nullptr,
		nullptr,
		"vertexMain",
		"vs_5_1",
		compileFlags,
		0,
		&m_vertexShader,
		nullptr
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to compile vertex shader.");
	}

	// Compile pixel shader
	hr = D3DCompileFromFile(
		pixelPath.c_str(),
		nullptr,
		nullptr,
		"fragmentMain",
		"ps_5_1",
		compileFlags,
		0,
		&m_pixelShader,
		nullptr
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to compile pixel shader.");
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
	inputElementDescs.reserve(layout.size());
	for (const auto& l : layout)
	{
		D3D12_INPUT_ELEMENT_DESC desc;
		desc.SemanticName = l.semanticName;
		desc.SemanticIndex = l.semanticIndex;
		desc.Format = l.format;
		desc.InputSlot = l.inputSlot;
		desc.AlignedByteOffset = l.offset;
		desc.InputSlotClass = l.inputClassification;
		desc.InstanceDataStepRate = 0;
		inputElementDescs.push_back(desc);
	}

	// Create Pipeline State Object (PSO) description
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs.data(), static_cast<UINT>(inputElementDescs.size()) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE{ m_vertexShader.Get() };
	psoDesc.PS = CD3DX12_SHADER_BYTECODE{ m_pixelShader.Get() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = primitiveTopology;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// Create the Pipeline State Object (PSO)
	hr = device->CreateGraphicsPipelineState(&psoDesc,IID_PPV_ARGS(&m_pipelineState));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create graphics pipeline state.");
	}
}
