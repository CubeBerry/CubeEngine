//Author: JEYOON YU
//Project: CubeEngine
//File: DXIndexBuffer.cpp
#include "DXIndexBuffer.hpp"
#include "DXInit.hpp"
#include <iostream>
#include <directx/d3dx12_resource_helpers.h>

DXIndexBuffer::DXIndexBuffer(const ComPtr<ID3D12Device>& device, std::vector<uint32_t>* indices)
{
	InitIndexBuffer(device, indices);
}

void DXIndexBuffer::InitIndexBuffer(const ComPtr<ID3D12Device>& device, std::vector<uint32_t>* indices)
{
	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * indices->size()),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create committed resource for index buffer.");
	}

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<const void*>(indices->data());
	indexData.RowPitch = sizeof(uint32_t) * indices->size();
}
