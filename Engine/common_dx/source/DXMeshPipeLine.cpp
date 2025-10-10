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
#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> errorMessages;

	// Compile mesh shader
	HRESULT hr = D3DCompileFromFile(
		meshPath.c_str(),
		nullptr,
		nullptr,
		"meshMain",
		"ms_6_5",
		compileFlags,
		0,
		&m_meshShader,
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
		pixelPath.c_str(),
		nullptr,
		nullptr,
		"fragmentMain",
		"ps_6_5",
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
	psoDesc.MS = CD3DX12_SHADER_BYTECODE{ m_meshShader.Get() };
	psoDesc.PS = CD3DX12_SHADER_BYTECODE{ m_pixelShader.Get() };
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.RasterizerState = desc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = isDepth ? TRUE : FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.PrimitiveTopologyType = primitiveTopology;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.SampleDesc = sampleDesc;

	auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);

	// Create Mesh Pipeline State Object (PSO)
	DXHelper::ThrowIfFailed(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
}
