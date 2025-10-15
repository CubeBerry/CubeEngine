//Author: JEYOON YU
//Project: CubeEngine
//File: DXStructuredBuffer.hpp
#pragma once
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

template <typename T>
class DXStructuredBuffer
{
public:
	DXStructuredBuffer(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::vector<T>& data,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);
	~DXStructuredBuffer() = default;

	DXStructuredBuffer(const DXStructuredBuffer&) = delete;
	DXStructuredBuffer& operator=(const DXStructuredBuffer&) = delete;
	DXStructuredBuffer(const DXStructuredBuffer&&) = delete;
	DXStructuredBuffer& operator=(const DXStructuredBuffer&&) = delete;

	ComPtr<ID3D12Resource>* GetStructuredBuffer() { return &m_sturucturedBuffer; }
private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue{ 1 };

	ComPtr<ID3D12Resource> m_sturucturedBuffer;
	ComPtr<ID3D12Resource> m_uploadBuffer;
};
