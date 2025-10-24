//Author: JEYOON YU
//Project: CubeEngine
//File: DXVertexBuffer.hpp
#pragma once
#include <directx/d3dx12.h>
#include "DXHelper.hpp"

using Microsoft::WRL::ComPtr;

class DXVertexBuffer
{
public:
	DXVertexBuffer(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		UINT strideSize, UINT totalSize, const void* data) : m_commandQueue(commandQueue)
	{
		InitVertexBuffer(device, strideSize, totalSize, data);
	}
	~DXVertexBuffer() = default;

	DXVertexBuffer(const DXVertexBuffer&) = delete;
	DXVertexBuffer& operator=(const DXVertexBuffer&) = delete;
	DXVertexBuffer(const DXVertexBuffer&&) = delete;
	DXVertexBuffer& operator=(const DXVertexBuffer&&) = delete;

	void InitVertexBuffer(const ComPtr<ID3D12Device>& device,
		UINT strideSize, UINT totalSize, const void* data);

	//ComPtr<ID3D12Resource>* GetVertexBuffer() { return &m_vertexBuffer; }
	D3D12_VERTEX_BUFFER_VIEW GetView() const { return m_vertexBufferView; }
private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue{ 1 };

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_uploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};
