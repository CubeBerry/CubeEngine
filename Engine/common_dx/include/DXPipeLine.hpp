//Author: JEYOON YU
//Project: CubeEngine
//File: DXPipeLine.hpp
#pragma once
#include <d3dx12/d3dx12.h>
#include <filesystem>
#include <initializer_list>

using Microsoft::WRL::ComPtr;

struct DXAttributeLayout
{
	LPCSTR semanticName;
	UINT semanticIndex;
    DXGI_FORMAT format;
	UINT inputSlot = 0;
    uint32_t offset = 0;
	D3D12_INPUT_CLASSIFICATION inputClassification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
};

struct DXPipeLineDesc
{
	std::filesystem::path vertexPath;
	std::filesystem::path pixelPath;
	std::vector<DXAttributeLayout> layout;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

	D3D12_RENDER_TARGET_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT).RenderTarget[0];
	D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

//enum class POLYGON_MODE
//{
//	FILL = 0,
//	LINE = 1,
//};

class DXPipeLine
{
public:
	DXPipeLine(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12RootSignature>& rootSignature,
		const DXPipeLineDesc& desc
	);

	ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }
private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3DBlob> m_vertexShader;
	ComPtr<ID3DBlob> m_pixelShader;
};

class DXPipeLineBuilder
{
public:
	DXPipeLineBuilder(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature);

	DXPipeLineBuilder& SetShaders(const std::filesystem::path& vertexPath, const std::filesystem::path& pixelPath);
	DXPipeLineBuilder& SetLayout(std::initializer_list<DXAttributeLayout> layout);
	DXPipeLineBuilder& SetRasterizer(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode, bool isCCW);
	DXPipeLineBuilder& SetDepthStencil(bool isDepth, bool isDepthWrite);
	DXPipeLineBuilder& SetRenderTargets(const std::vector<DXGI_FORMAT>& rtvFormats);
	DXPipeLineBuilder& SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);
	DXPipeLineBuilder& SetBlendMode(const D3D12_RENDER_TARGET_BLEND_DESC& blendDesc);
	DXPipeLineBuilder& SetSampleDesc(UINT count, UINT quality);

	std::unique_ptr<DXPipeLine> Build();
private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	DXPipeLineDesc m_desc;
};
