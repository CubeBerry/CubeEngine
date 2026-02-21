//Author: JEYOON YU
//Project: CubeEngine
//File: DXPipeLine.cpp
#include "DXPipeLine.hpp"
#include "DXHelper.hpp"

#include <d3dcompiler.h>

DXPipeLine::DXPipeLine(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12RootSignature>& rootSignature,
	const DXPipeLineDesc& desc
)
{
#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> errorMessages;

	// Compile vertex shader
	HRESULT hr = D3DCompileFromFile(
		desc.vertexPath.c_str(),
		nullptr,
		nullptr,
		"vertexMain",
		"vs_5_1",
		compileFlags,
		0,
		&m_vertexShader,
		&errorMessages
	);
	if (FAILED(hr))
	{
		if (errorMessages)
		{
			const char* string = static_cast<const char*>(errorMessages->GetBufferPointer());
			OutputDebugStringA("Vertex Shader Compilation Error:\n");
			OutputDebugStringA(string);
		}
		throw std::runtime_error("Failed to compile vertex shader.");
	}

	// Compile pixel shader
	hr = D3DCompileFromFile(
		desc.pixelPath.c_str(),
		nullptr,
		nullptr,
		"fragmentMain",
		"ps_5_1",
		compileFlags,
		0,
		&m_pixelShader,
		&errorMessages
	);
	if (FAILED(hr))
	{
		if (errorMessages)
		{
			const char* string = static_cast<const char*>(errorMessages->GetBufferPointer());
			OutputDebugStringA("Pixel Shader Compilation Error:\n");
			OutputDebugStringA(string);
		}
		throw std::runtime_error("Failed to compile pixel shader.");
	}

	//auto vertexShader = DXHelper::ReadShaderFile(vertexPath);
	//auto pixelShader = DXHelper::ReadShaderFile(pixelPath);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
	inputElementDescs.reserve(desc.layout.size());
	for (const auto& l : desc.layout)
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

	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE{ m_vertexShader.Get() };
	psoDesc.PS = CD3DX12_SHADER_BYTECODE{ m_pixelShader.Get() };
	//psoDesc.VS = CD3DX12_SHADER_BYTECODE{ vertexShader.data(), vertexShader.size() };
	//psoDesc.PS = CD3DX12_SHADER_BYTECODE{ pixelShader.data(), pixelShader.size() };

	D3D12_BLEND_DESC finalBlendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	finalBlendDesc.RenderTarget[0] = desc.blendDesc;
	psoDesc.BlendState = finalBlendDesc;
	psoDesc.SampleMask = UINT_MAX;

	// DirectX basically uses Left-Handed Coordinate System but this makes DirectX use Right-Handed Coordinate System
	psoDesc.RasterizerState = desc.rasterizerDesc;

	psoDesc.DepthStencilState = desc.depthStencilDesc;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	psoDesc.InputLayout = { inputElementDescs.data(), static_cast<UINT>(inputElementDescs.size()) };
	psoDesc.PrimitiveTopologyType = desc.primitiveTopology;

	psoDesc.NumRenderTargets = static_cast<UINT>(desc.rtvFormats.size());
	for (size_t i = 0; i < desc.rtvFormats.size(); ++i) psoDesc.RTVFormats[i] = desc.rtvFormats[i];
	psoDesc.DSVFormat = desc.depthStencilDesc.DepthEnable ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc = desc.sampleDesc;

	// Create Pipeline State Object (PSO)
	DXHelper::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	DXHelper::ThrowIfFailed(m_pipelineState->SetName(L"PSO"));
}

DXPipeLineBuilder::DXPipeLineBuilder(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature)
	: m_device(device), m_rootSignature(rootSignature)
{
}

DXPipeLineBuilder& DXPipeLineBuilder::SetShaders(const std::filesystem::path& vertexPath, const std::filesystem::path& pixelPath)
{
	m_desc.vertexPath = vertexPath;
	m_desc.pixelPath = pixelPath;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetLayout(std::initializer_list<DXAttributeLayout> layout)
{
	m_desc.layout = layout;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetRasterizer(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode, bool isCCW)
{
	m_desc.rasterizerDesc.FillMode = fillMode;
	m_desc.rasterizerDesc.CullMode = cullMode;
	m_desc.rasterizerDesc.FrontCounterClockwise = isCCW ? TRUE : FALSE;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetDepthStencil(bool isDepth, bool isDepthWrite)
{
	m_desc.depthStencilDesc.DepthEnable = isDepth ? TRUE : FALSE;
	m_desc.depthStencilDesc.DepthWriteMask = isDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetRenderTargets(const std::vector<DXGI_FORMAT>& rtvFormats)
{
	m_desc.rtvFormats = rtvFormats;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
{
	m_desc.primitiveTopology = topology;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetBlendMode(const D3D12_RENDER_TARGET_BLEND_DESC& blendDesc)
{
	m_desc.blendDesc = blendDesc;
	return *this;
}

DXPipeLineBuilder& DXPipeLineBuilder::SetSampleDesc(UINT count, UINT quality)
{
	m_desc.sampleDesc.Count = count;
	m_desc.sampleDesc.Quality = quality;
	return *this;
}

std::unique_ptr<DXPipeLine> DXPipeLineBuilder::Build()
{
	return std::make_unique<DXPipeLine>(m_device, m_rootSignature, m_desc);
}
