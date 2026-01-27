//Author: JEYOON YU
//Project: CubeEngine
//File: DXMeshPipeLine.cpp
#include "DXMeshPipeLine.hpp"
#include "DXHelper.hpp"

#include <d3dcompiler.h>

DXMeshPipeLine::DXMeshPipeLine(
	const ComPtr<ID3D12Device2>& device,
	const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& meshPath, const std::filesystem::path& pixelPath,
	D3D12_FILL_MODE fillMode,
	D3D12_CULL_MODE cullMode,
	const DXGI_SAMPLE_DESC& sampleDesc,
	bool isCCW,
	bool isDepth,
	DXGI_FORMAT rtvFormat,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology
	)
{
	ComPtr<ID3DBlob> errorMessages;

	std::vector<char> meshShader = DXHelper::ReadShaderFile(meshPath);
	std::vector<char> pixelShader = DXHelper::ReadShaderFile(pixelPath);

	// Create Mesh Pipeline State Object(PSO) description
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};

	D3D12_RASTERIZER_DESC& desc = psoDesc.RasterizerState;
	desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.FillMode = fillMode;
	desc.CullMode = cullMode;
	// DirectX basically uses Left-Handed Coordinate System but this makes DirectX use Right-Handed Coordinate System
	desc.FrontCounterClockwise = isCCW ? TRUE : FALSE;

	psoDesc.pRootSignature = rootSignature.Get();
	//psoDesc.AS
	psoDesc.MS = CD3DX12_SHADER_BYTECODE{ meshShader.data(), meshShader.size() };
	psoDesc.PS = CD3DX12_SHADER_BYTECODE{ pixelShader.data(), pixelShader.size() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = desc;
	psoDesc.DepthStencilState.DepthEnable = isDepth ? TRUE : FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.PrimitiveTopologyType = primitiveTopology;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = sampleDesc;

	auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);

	// Create Mesh Pipeline State Object (PSO)
	DXHelper::ThrowIfFailed(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
}

DXMeshPipeLine::DXMeshPipeLine(
	const ComPtr<ID3D12Device2>& device,
	const ComPtr<ID3D12RootSignature>& rootSignature,
	const std::filesystem::path& meshPath, const std::filesystem::path& pixelPath,
	D3D12_FILL_MODE fillMode,
	D3D12_CULL_MODE cullMode,
	const DXGI_SAMPLE_DESC& sampleDesc,
	bool isCCW,
	bool isDepth,
	const std::vector<DXGI_FORMAT>& rtvFormats,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology
)
{
	ComPtr<ID3DBlob> errorMessages;

	std::vector<char> meshShader = DXHelper::ReadShaderFile(meshPath);
	std::vector<char> pixelShader = DXHelper::ReadShaderFile(pixelPath);

	// Create Mesh Pipeline State Object(PSO) description
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};

	D3D12_RASTERIZER_DESC& desc = psoDesc.RasterizerState;
	desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.FillMode = fillMode;
	desc.CullMode = cullMode;
	// DirectX basically uses Left-Handed Coordinate System but this makes DirectX use Right-Handed Coordinate System
	desc.FrontCounterClockwise = isCCW ? TRUE : FALSE;

	psoDesc.pRootSignature = rootSignature.Get();
	//psoDesc.AS
	psoDesc.MS = CD3DX12_SHADER_BYTECODE{ meshShader.data(), meshShader.size() };
	psoDesc.PS = CD3DX12_SHADER_BYTECODE{ pixelShader.data(), pixelShader.size() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = desc;
	psoDesc.DepthStencilState.DepthEnable = isDepth ? TRUE : FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.PrimitiveTopologyType = primitiveTopology;
	psoDesc.NumRenderTargets = static_cast<UINT>(rtvFormats.size());
	for (size_t i = 0; i < rtvFormats.size(); ++i) psoDesc.RTVFormats[i] = rtvFormats[i];
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = sampleDesc;

	auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);

	// Create Mesh Pipeline State Object (PSO)
	DXHelper::ThrowIfFailed(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
}
