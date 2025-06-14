//Author: JEYOON YU
//Project: CubeEngine
//File: DXVertexBuffer.hpp
#pragma once
#include <directx/d3dx12_core.h>
#include <d3d12.h>
#include <wrl.h>

#include <stdexcept>

using Microsoft::WRL::ComPtr;

class DXVertexBuffer
{
public:
	DXVertexBuffer(const ComPtr<ID3D12Device>& device, UINT strideSize, UINT totalSize, const void* data)
	{
		InitVertexBuffer(device, strideSize, totalSize, data);
	}
	~DXVertexBuffer() = default;

	void InitVertexBuffer(const ComPtr<ID3D12Device>& device, UINT strideSize, UINT totalSize, const void* data)
	{
		HRESULT hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(totalSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create committed resource for vertex buffer.");
		}

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		hr = m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to map vertex buffer.");
		}
		memcpy(pVertexDataBegin, data, totalSize);
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = strideSize;
		m_vertexBufferView.SizeInBytes = totalSize;
	}

	//ComPtr<ID3D12Resource>* GetVertexBuffer() { return &m_vertexBuffer; }
	D3D12_VERTEX_BUFFER_VIEW GetView() const { return m_vertexBufferView; }
private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};
