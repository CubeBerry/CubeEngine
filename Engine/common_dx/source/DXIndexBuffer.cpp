//Author: JEYOON YU
//Project: CubeEngine
//File: DXIndexBuffer.cpp
#include "DXIndexBuffer.hpp"
#include "DXHelper.hpp"

#include <d3d12.h>

#include <iostream>
#include <stdexcept>

DXIndexBuffer::DXIndexBuffer(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const std::vector<uint32_t>* indices) : m_commandQueue(commandQueue)
{
	InitIndexBuffer(device, indices);
}

void DXIndexBuffer::InitIndexBuffer(const ComPtr<ID3D12Device>& device, const std::vector<uint32_t>* indices)
{
	std::wstring targetName{ L"Index Buffer" };
	DXHelper::CreateFenceSet(device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC defaultResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * indices->size());
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&defaultResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	));
	m_indexBuffer->SetName(L"Default Index Buffer Resource");

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * indices->size());

	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_uploadBuffer)
	));
	m_uploadBuffer->SetName(L"Upload Index Buffer Resource");

	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	DXHelper::ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indices->data(), sizeof(uint32_t) * indices->size());
	m_uploadBuffer->Unmap(0, nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->CopyBufferRegion(
		m_indexBuffer.Get(),
		0,
		m_uploadBuffer.Get(),
		0,
		sizeof(uint32_t) * indices->size()
	);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER
	);
	m_commandList->ResourceBarrier(1, &barrier);

	// Initialize the index buffer view
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices->size());
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	DXHelper::ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// WaitForGPU
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
	DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));

	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValue++;
}
