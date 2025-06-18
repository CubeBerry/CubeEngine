//Author: JEYOON YU
//Project: CubeEngine
//File: DXSkybox.cpp
#include "DXTexture.hpp"
#include "DXSkybox.hpp"
#include <iostream>
#include <filesystem>

#include "DXPipeLine.hpp"
#include "DXVertexBuffer.hpp"

DXSkybox::DXSkybox(const ComPtr<ID3D12Device>& device,
                   const ComPtr<ID3D12CommandQueue>& commandQueue,
                   const std::filesystem::path& path)
{
	// Create Command Allocator
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create texture command allocator.");
	}
	m_commandAllocator->SetName(L"Skybox Texture Command Allocator");

	// Create Command List
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create texture command list.");
	}
	m_commandList->SetName(L"Skybox Texture Command List");
	m_commandList->Close();

	// Create Fence
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create texture fence.");
	}
	m_fence->SetName(L"Skybox Texture Fence");

	// Create Fence Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		throw std::runtime_error("Failed to create texture fence event.");
	}

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 3;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create SRV descriptor heap.");
	}
	m_srvHeap->SetName(L"Skybox Resource View Heap");
	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_equirectangularMap = std::make_unique<DXTexture>();
	m_equirectangularMap->LoadTexture(device, m_commandList, m_srvHeap, commandQueue, m_fence, m_fenceEvent, true, path, "skybox", true);
	faceSize = m_equirectangularMap->GetHeight();

	EquirectangularToCube();
	CalculateIrradiance();
	PrefilteredEnvironmentMap();
	BRDFLUT();
}

DXSkybox::~DXSkybox()
{
	//m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
	//HRESULT hr = m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	//if (FAILED(hr))
	//{
	//	throw std::runtime_error("Failed to set event on fence completion.");
	//}
	//WaitForSingleObject(m_fenceEvent, INFINITE);
	//CloseHandle(m_fenceEvent);
}

void DXSkybox::ExecuteCommandList()
{
	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Wait for GPU
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
	m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValue++;
}

void DXSkybox::EquirectangularToCube()
{
	// Create Cubemap Resource
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT64>(faceSize);
	texDesc.Height = static_cast<UINT>(faceSize);
	texDesc.DepthOrArraySize = 6;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_cubemap)
	);
	m_cubemap->SetName(L"Skybox Cubemap Resource");

	// Create Render Target View (RTV) heap for each face
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < 6; ++i)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;
		m_device->CreateRenderTargetView(m_cubemap.Get(), &rtvDesc, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}

	// Prepare Pipeline
	ComPtr<ID3D12RootSignature> rootSignature;
	CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	// Create Pipeline State Object (PSO)
	struct VA
	{
		glm::vec3 position;
	};

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VA, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	std::unique_ptr<DXPipeLine> pipeline = std::make_unique<DXPipeLine>(
		m_device,
		rootSignature,
		"../Engine/shaders/hlsl/Cubemap.vert.hlsl",
		"../Engine/shaders/hlsl/Equirectangular.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{ positionLayout }
	);

	// Record Command List
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), pipeline->GetPipelineState().Get());

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(faceSize), static_cast<FLOAT>(faceSize), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(faceSize), static_cast<LONG>(faceSize) };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);
	m_commandList->SetGraphicsRootSignature(rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	DXVertexBuffer cubeVertexBuffer{ m_device, sizeof(glm::vec3), static_cast<UINT>(sizeof(glm::vec3) * m_skyboxVertices.size()), m_skyboxVertices.data() };

	//struct WorldToNDC { views, projection };

	D3D12_VERTEX_BUFFER_VIEW vbv = cubeVertexBuffer.GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbv);
}

void DXSkybox::CalculateIrradiance()
{
}

void DXSkybox::PrefilteredEnvironmentMap()
{
}

void DXSkybox::BRDFLUT()
{
}
