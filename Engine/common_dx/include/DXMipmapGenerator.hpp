//Author: JEYOON YU
//Project: CubeEngine
//File: DXMipmapGenerator.hpp
#pragma once
#define NOMINMAX
#include <d3dx12/d3dx12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DXMipmapGenerator
{
public:
	DXMipmapGenerator(const ComPtr<ID3D12Device>& device);
	~DXMipmapGenerator() = default;

	DXMipmapGenerator(const DXMipmapGenerator&) = delete;
	DXMipmapGenerator& operator=(const DXMipmapGenerator&) = delete;
	DXMipmapGenerator(const DXMipmapGenerator&&) = delete;
	DXMipmapGenerator& operator=(const DXMipmapGenerator&&) = delete;

	void Generate(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& texture) const;
private:
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	UINT m_descriptorSize;
};
