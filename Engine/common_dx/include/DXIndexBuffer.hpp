//Author: JEYOON YU
//Project: CubeEngine
//File: DXIndexBuffer.hpp
#pragma once
#include <directx/d3dx12_core.h>
#include <wrl.h>

#include <vector>

using Microsoft::WRL::ComPtr;

class DXIndexBuffer
{
public:
	DXIndexBuffer(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::vector<uint32_t>* indices);
	~DXIndexBuffer() = default;

	DXIndexBuffer(const DXIndexBuffer&) = delete;
	DXIndexBuffer& operator=(const DXIndexBuffer&) = delete;
	DXIndexBuffer(const DXIndexBuffer&&) = delete;
	DXIndexBuffer& operator=(const DXIndexBuffer&&) = delete;

	void InitIndexBuffer(const ComPtr<ID3D12Device>& device, const std::vector<uint32_t>* indices);

	//ComPtr<ID3D12Resource>* GetIndexBuffer() { return &m_indexBuffer; }
	D3D12_INDEX_BUFFER_VIEW GetView() const { return m_indexBufferView; }
private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue{ 1 };

	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_uploadBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};
