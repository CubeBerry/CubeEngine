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
		UINT strideSize, UINT totalSize, const void* data)
	{
		std::wstring targetName{ L"Vertex Buffer" };
		DXHelper::CreateFenceSet(device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

		DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
		DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC defaultResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
		DXHelper::ThrowIfFailed(device->CreateCommittedResource(
			&defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&defaultResourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)
		));
		m_vertexBuffer->SetName(L"Default Vertex Buffer Resource");

		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
		DXHelper::ThrowIfFailed(device->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadBuffer)
		));
		m_uploadBuffer->SetName(L"Upload Vertex Buffer Resource");

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		DXHelper::ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, data, totalSize);
		m_uploadBuffer->Unmap(0, nullptr);

		m_commandList->CopyBufferRegion(
			m_vertexBuffer.Get(),
			0,
			m_uploadBuffer.Get(),
			0,
			totalSize
		);

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		m_commandList->ResourceBarrier(1, &barrier);

		// Initialize the vertex buffer view
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = strideSize;
		m_vertexBufferView.SizeInBytes = totalSize;

		DXHelper::ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// WaitForGPU
		DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
		DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));

		WaitForSingleObject(m_fenceEvent, INFINITE);
		m_fenceValue++;
	}

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
