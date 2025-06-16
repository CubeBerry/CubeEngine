//Author: JEYOON YU
//Project: CubeEngine
//File: DXPipeLine.hpp
#pragma once
#include <directx/d3dx12.h>

#include <initializer_list>
#include <filesystem>

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
		const std::filesystem::path& vertexPath, const std::filesystem::path& pixelPath,
		std::initializer_list<DXAttributeLayout> layout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);
	~DXPipeLine() = default;

	ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }
private:
	ComPtr<ID3DBlob> m_vertexShader;
	ComPtr<ID3DBlob> m_pixelShader;
	ComPtr<ID3D12PipelineState> m_pipelineState;
};