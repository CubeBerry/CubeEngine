//Author: JEYOON YU
//Project: CubeEngine
//File: DXIndexBuffer.hpp
#pragma once
#include <directx/d3dx12_core.h>
#include <d3d12.h>
#include <wrl.h>

#include <stdexcept>
#include <vector>

using Microsoft::WRL::ComPtr;

class DXIndexBuffer
{
public:
	DXIndexBuffer(const ComPtr<ID3D12Device>& device, std::vector<uint32_t>* indices);
	~DXIndexBuffer() = default;

	void InitIndexBuffer(const ComPtr<ID3D12Device>& device, std::vector<uint32_t>* indices);

	//ComPtr<ID3D12Resource>* GetIndexBuffer() { return &m_indexBuffer; }
	D3D12_VERTEX_BUFFER_VIEW GetView() const { return m_indexBufferView; }
private:
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_indexBufferView;
};
