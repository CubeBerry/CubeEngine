//Author: JEYOON YU
//Project: CubeEngine
//File: DXConstantBuffer.hpp
#pragma once
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

// Align constant buffer size to D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT = 256 bytes
inline UINT AlignConstantBufferSize(size_t size)
{
	size_t alignedSize = (size + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
	return static_cast<UINT>(alignedSize);
}

template<typename Type>
class DXConstantBuffer
{
public:
	DXConstantBuffer(const ComPtr<ID3D12Device>& device);
	~DXConstantBuffer();

	DXConstantBuffer(const DXConstantBuffer&) = delete;
	DXConstantBuffer& operator=(const DXConstantBuffer&) = delete;
	DXConstantBuffer(const DXConstantBuffer&&) = delete;
	DXConstantBuffer& operator=(const DXConstantBuffer&&) = delete;

	void InitConstantBuffer(const ComPtr<ID3D12Device>& device);
	void UpdateConstant(const void* data, const uint32_t frameIndex) const;

	ComPtr<ID3D12Resource> GetConstantBuffer() const { return m_constantBuffer; }
private:
	ComPtr<ID3D12Resource> m_constantBuffer;
	void* m_mappedData{ nullptr };
	UINT m_alignedSize{ 0 };
};

template<typename Type>
DXConstantBuffer<Type>::DXConstantBuffer(const ComPtr<ID3D12Device>& device)
{
	InitConstantBuffer(device);
}

template<typename Type>
DXConstantBuffer<Type>::~DXConstantBuffer()
{
	if (m_constantBuffer)
	{
		m_constantBuffer->Unmap(0, nullptr);
	}
	m_mappedData = nullptr;
}

template<typename Type>
void DXConstantBuffer<Type>::InitConstantBuffer(const ComPtr<ID3D12Device>& device)
{
	m_alignedSize = AlignConstantBufferSize(sizeof(Type));

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_alignedSize);

	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer)
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create committed resource for constant buffer.");
	}

	CD3DX12_RANGE readRange(0, 0);
	hr = m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedData));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to map constant buffer.");
	}
}

template<typename Type>
void DXConstantBuffer<Type>::UpdateConstant(const void* data, const uint32_t frameIndex) const
{
	memcpy(m_mappedData, data, sizeof(Type));
}
