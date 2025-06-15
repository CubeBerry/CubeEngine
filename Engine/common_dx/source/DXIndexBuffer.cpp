//Author: JEYOON YU
//Project: CubeEngine
//File: DXIndexBuffer.cpp
#include "DXIndexBuffer.hpp"

#include <directx/d3dx12_resource_helpers.h>
#include <d3d12.h>

#include <iostream>
#include <stdexcept>

DXIndexBuffer::DXIndexBuffer(const ComPtr<ID3D12Device>& device, std::vector<uint32_t>* indices)
{
	InitIndexBuffer(device, indices);
}

//@ TODO Implement UPLOAD & DEFAULT heap support
void DXIndexBuffer::InitIndexBuffer(const ComPtr<ID3D12Device>& device, const std::vector<uint32_t>* indices)
{
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * indices->size());

	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create committed resource for index buffer.");
	}

	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	hr = m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to map index buffer.");
	}
	memcpy(pIndexDataBegin, indices->data(), sizeof(uint32_t) * indices->size());
	m_indexBuffer->Unmap(0, nullptr);

	// Initialize the index buffer view
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices->size());
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}
