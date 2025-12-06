//Author: JEYOON YU
//Project: CubeEngine
//File: DXVertexBuffer.cpp
#include "DXVertexBuffer.hpp"

#include "DXHelper.hpp"
#include "DXInitializer.hpp"

void DXVertexBuffer::InitVertexBuffer(const ComPtr<ID3D12Device>& device,
	UINT strideSize, UINT totalSize, const void* data)
{
	std::wstring targetName{ L"Vertex Buffer" };
	DXInitializer::CreateFenceSet(device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	m_vertexBuffer = DXInitializer::CreateBufferResource(
		device,
		totalSize,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_COMMON
	);
	m_vertexBuffer->SetName(L"Default Vertex Buffer Resource");

	m_uploadBuffer = DXInitializer::CreateBufferResource(
		device,
		totalSize,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ
	);
	m_uploadBuffer->SetName(L"Upload Vertex Buffer Resource");

	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	DXHelper::ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, data, totalSize);
	m_uploadBuffer->Unmap(0, nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->CopyBufferRegion(
		m_vertexBuffer.Get(),
		0,
		m_uploadBuffer.Get(),
		0,
		totalSize
	);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
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
