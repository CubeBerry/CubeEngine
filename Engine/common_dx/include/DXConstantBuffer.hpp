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

// Constant Buffer operates using the concept of a ring buffer
template<typename Type>
class DXConstantBuffer
{
public:
	DXConstantBuffer(const ComPtr<ID3D12Device>& device, const UINT& frameCount);
	~DXConstantBuffer();

	DXConstantBuffer(const DXConstantBuffer&) = delete;
	DXConstantBuffer& operator=(const DXConstantBuffer&) = delete;
	DXConstantBuffer(const DXConstantBuffer&&) = delete;
	DXConstantBuffer& operator=(const DXConstantBuffer&&) = delete;

	void UpdateConstant(const void* data, const uint32_t frameIndex) const;

	[[nodiscard]] ComPtr<ID3D12Resource> GetConstantBuffer() const { return m_constantBuffer; }
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(const uint32_t& frameIndex) const
	{
		return m_constantBuffer->GetGPUVirtualAddress() + (static_cast<uint64_t>(frameIndex) * m_alignedSizePerFrame);
	}
private:
	ComPtr<ID3D12Resource> m_constantBuffer;
	uint8_t* m_mappedData{ nullptr };
	UINT m_alignedSizePerFrame{ 0 };
};

template<typename Type>
DXConstantBuffer<Type>::DXConstantBuffer(const ComPtr<ID3D12Device>& device, const UINT& frameCount)
{
	m_alignedSizePerFrame = AlignConstantBufferSize(sizeof(Type));
	const UINT& totalBufferSize = m_alignedSizePerFrame * frameCount;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBufferSize);

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
DXConstantBuffer<Type>::~DXConstantBuffer()
{
	if (m_constantBuffer)
	{
		m_constantBuffer->Unmap(0, nullptr);
	}
	m_mappedData = nullptr;
}

template<typename Type>
void DXConstantBuffer<Type>::UpdateConstant(const void* data, const uint32_t frameIndex) const
{
	uint8_t* dest = m_mappedData + (static_cast<uint64_t>(frameIndex) * m_alignedSizePerFrame);
	memcpy(dest, data, sizeof(Type));
}
