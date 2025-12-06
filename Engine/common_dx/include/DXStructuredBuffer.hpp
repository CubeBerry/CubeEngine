//Author: JEYOON YU
//Project: CubeEngine
//File: DXStructuredBuffer.hpp
#pragma once
#include <d3dx12/d3dx12.h>

#include "DXHelper.hpp"
#include "DXInitializer.hpp"

using Microsoft::WRL::ComPtr;

template <typename T>
class DXStructuredBuffer
{
public:
	// SRV
	DXStructuredBuffer(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::vector<T>& data,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);
	// SRV + UAV
	DXStructuredBuffer(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::vector<T>& data,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE& uavHandle);
	~DXStructuredBuffer() = default;

	DXStructuredBuffer(const DXStructuredBuffer&) = delete;
	DXStructuredBuffer& operator=(const DXStructuredBuffer&) = delete;
	DXStructuredBuffer(const DXStructuredBuffer&&) = delete;
	DXStructuredBuffer& operator=(const DXStructuredBuffer&&) = delete;

	ID3D12Resource* GetStructuredBuffer() { return m_structuredBuffer.Get(); }
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{
		return m_structuredBuffer->GetGPUVirtualAddress();
	}
private:
	void Initialize(
		const ComPtr<ID3D12Device>& device,
		const std::vector<T>& data,
		D3D12_RESOURCE_FLAGS resourceFlags,
		D3D12_RESOURCE_STATES finalState);

	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue{ 1 };

	ComPtr<ID3D12Resource> m_structuredBuffer;
	ComPtr<ID3D12Resource> m_uploadBuffer;
};

template <typename T>
void DXStructuredBuffer<T>::Initialize(
	const ComPtr<ID3D12Device>& device,
	const std::vector<T>& data,
	D3D12_RESOURCE_FLAGS resourceFlags,
	D3D12_RESOURCE_STATES finalState)
{
	UINT strideSize = sizeof(T);
	UINT bufferSize = static_cast<UINT>(data.size() * strideSize);

	std::wstring targetName{ L"Structured Buffer" };
	DXInitializer::CreateFenceSet(device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	m_structuredBuffer = DXInitializer::CreateBufferResource(
		device,
		bufferSize,
		resourceFlags,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_COMMON
	);
	m_structuredBuffer->SetName(L"Default Structured Buffer Resource");

	m_uploadBuffer = DXInitializer::CreateBufferResource(
		device,
		bufferSize,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ
	);
	m_uploadBuffer->SetName(L"Upload Structured Buffer Resource");

	UINT8* pStructuredDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	DXHelper::ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pStructuredDataBegin)));
	memcpy(pStructuredDataBegin, data.data(), bufferSize);
	m_uploadBuffer->Unmap(0, nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_structuredBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->CopyBufferRegion(
		m_structuredBuffer.Get(),
		0,
		m_uploadBuffer.Get(),
		0,
		bufferSize
	);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_structuredBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		finalState
	);
	m_commandList->ResourceBarrier(1, &barrier);

	DXHelper::ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// WaitForGPU
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
	DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));

	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValue++;
}

// SRV
template <typename T>
DXStructuredBuffer<T>::DXStructuredBuffer(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const std::vector<T>& data,
	const CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) : m_commandQueue(commandQueue)
{
	Initialize(device, data, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

	// Initialize the Shader Resource View (SRV)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	//srvDesc.Buffer.NumElements = static_cast<UINT>(m_buffer->GetDesc().Width / sizeof(T));
	//srvDesc.Buffer.NumElements = bufferSize / strideSize;
	srvDesc.Buffer.NumElements = static_cast<UINT>(data.size());
	//srvDesc.Buffer.StructureByteStride = strideSize;
	srvDesc.Buffer.StructureByteStride = sizeof(T);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_structuredBuffer.Get(), &srvDesc, srvHandle);
}

// SRV + UAV
template <typename T>
DXStructuredBuffer<T>::DXStructuredBuffer(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const std::vector<T>& data,
	const CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle,
	const CD3DX12_CPU_DESCRIPTOR_HANDLE& uavHandle) : m_commandQueue(commandQueue)
{
	Initialize(device, data, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	UINT numElements = static_cast<UINT>(data.size());
	UINT stride = sizeof(T);

	// Initialize the Shader Resource View (SRV)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	//srvDesc.Buffer.NumElements = static_cast<UINT>(m_buffer->GetDesc().Width / sizeof(T));
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = stride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_structuredBuffer.Get(), &srvDesc, srvHandle);

	// Initialize the Unordered Access View (UAV)
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numElements;
	uavDesc.Buffer.StructureByteStride = stride;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(m_structuredBuffer.Get(), nullptr, &uavDesc, uavHandle);
}
