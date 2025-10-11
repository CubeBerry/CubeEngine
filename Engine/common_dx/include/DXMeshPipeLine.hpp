//Author: JEYOON YU
//Project: CubeEngine
//File: DXMeshPipeLine.hpp
#pragma once
#include <directx/d3dx12.h>

#include <filesystem>

using Microsoft::WRL::ComPtr;

// https://jcgt.org/published/0012/02/01/paper-lowres.pdf
// https://github.com/Senbyo/meshletmaker
// 1. NVIDIA Meshlet Builder
// https://github.com/nvpro-samples/gl_vk_meshlet_cadscene
// 2. Meshlet Creation Library
// https://github.com/zeux/meshoptimizer
class DXMeshPipeLine
{
public:
	DXMeshPipeLine(
		const ComPtr<ID3D12Device2>& device,
		const ComPtr<ID3D12RootSignature>& rootSignature,
		const std::filesystem::path& meshPath, const std::filesystem::path& pixelPath,
		D3D12_FILL_MODE fillMode,
		D3D12_CULL_MODE cullMode,
		const DXGI_SAMPLE_DESC& sampleDesc,
		bool isCCW,
		bool isDepth,
		DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);
	~DXMeshPipeLine() = default;

	ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }
private:
	ComPtr<ID3DBlob> m_meshShader;
	ComPtr<ID3DBlob> m_pixelShader;
	ComPtr<ID3D12PipelineState> m_pipelineState;
};