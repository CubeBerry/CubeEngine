//Author: JEYOON YU
//Project: CubeEngine
//File: DXTexture.cpp
#include "DXTexture.hpp"
#include "DXHelper.hpp"
#include <iostream>

#include "stb-master/stb_image.h"

void DXTexture::LoadTexture(
	const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12DescriptorHeap>& srvHeap,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const ComPtr<ID3D12Fence>& fence,
	const HANDLE& fenceEvent,
	const INT& offsetIndex,
	bool isHDR, const std::filesystem::path& path_, std::string name_, bool flip)
{
	name = name_;

	if (flip) stbi_set_flip_vertically_on_load(true);

	auto path = path_;
	int texChannels;
	//Read in image file
	void* data{ nullptr };
	if (isHDR)
	{
		data = stbi_loadf(path.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
	}
	else
		data = stbi_load(path.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);

	ComPtr<ID3D12Resource> textureUploadHeap;

	{
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = isHDR ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = static_cast<UINT64>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		DXHelper::ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_texture)
		));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

		// Create the GPU upload buffer
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		DXHelper::ThrowIfFailed(device->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap)
		));

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = data;
		textureData.RowPitch = static_cast<INT64>(width) * (isHDR ? 16 : 4); // 16 bytes for R32G32B32A32_FLOAT, 4 bytes for R8G8B8A8_UNORM
		textureData.SlicePitch = textureData.RowPitch * static_cast<UINT>(height);

		UpdateSubresources(
			commandList.Get(),
			m_texture.Get(),
			textureUploadHeap.Get(),
			0,
			0,
			1,
			&textureData
		);

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &barrier);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = isHDR ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), offsetIndex, descriptorSize);

		device->CreateShaderResourceView(m_texture.Get(), &srvDesc, srvHandle);
	}

	DXHelper::ThrowIfFailed(commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	DXHelper::ThrowIfFailed(commandQueue->Signal(fence.Get(), 1));
	if (fence->GetCompletedValue() < 1)
	{
		DXHelper::ThrowIfFailed(fence->SetEventOnCompletion(1, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	//CloseHandle(fenceEvent);
}

void DXTexture::LoadSkyBox(bool isHDR, const std::filesystem::path& right, const std::filesystem::path& left, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& front, const std::filesystem::path& back)
{
	isHDR;
	right;
	left;
	top;
	bottom;
	front;
	back;
}
