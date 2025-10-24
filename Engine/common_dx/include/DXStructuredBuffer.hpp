//Author: JEYOON YU
//Project: CubeEngine
//File: DXStructuredBuffer.hpp
#pragma once
#include <directx/d3dx12.h>

#include "DXHelper.hpp"

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
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{
		return m_sturucturedBuffer->GetGPUVirtualAddress();
	}
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

template <typename T>
DXStructuredBuffer<T>::DXStructuredBuffer(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const std::vector<T>& data,
	const CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) : m_commandQueue(commandQueue)
{
	UINT strideSize = sizeof(T);
	UINT bufferSize = static_cast<UINT>(data.size() * strideSize);

	std::wstring targetName{ L"Structured Buffer" };
	DXHelper::CreateFenceSet(device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC defaultResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&defaultResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_sturucturedBuffer)
	));
	m_sturucturedBuffer->SetName(L"Default Structured Buffer Resource");

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	DXHelper::ThrowIfFailed(device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_uploadBuffer)
	));
	m_uploadBuffer->SetName(L"Upload Structured Buffer Resource");

	UINT8* pStructuredDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	DXHelper::ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pStructuredDataBegin)));
	memcpy(pStructuredDataBegin, data.data(), bufferSize);
	m_uploadBuffer->Unmap(0, nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_sturucturedBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->CopyBufferRegion(
		m_sturucturedBuffer.Get(),
		0,
		m_uploadBuffer.Get(),
		0,
		bufferSize
	);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_sturucturedBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	);
	m_commandList->ResourceBarrier(1, &barrier);

	DXHelper::ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Initialize the Shader Resource View (SRV)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	//srvDesc.Buffer.NumElements = static_cast<UINT>(m_buffer->GetDesc().Width / sizeof(T));
	srvDesc.Buffer.NumElements = bufferSize / strideSize;
	srvDesc.Buffer.StructureByteStride = strideSize;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_sturucturedBuffer.Get(), &srvDesc, srvHandle);

	// WaitForGPU
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
	DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));

	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValue++;
}
