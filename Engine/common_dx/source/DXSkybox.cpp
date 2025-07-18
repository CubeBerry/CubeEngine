//Author: JEYOON YU
//Project: CubeEngine
//File: DXSkybox.cpp
#include "DXSkybox.hpp"
#include <filesystem>

#include "DXHelper.hpp"
#include "DXTexture.hpp"
#include "DXConstantBuffer.hpp"

DXSkybox::DXSkybox(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const ComPtr<ID3D12DescriptorHeap>& srvHeap,
	const UINT& srvHeapStartOffset) : m_device(device), m_commandQueue(commandQueue), m_srvHeap(srvHeap), m_srvHeapStartOffset(srvHeapStartOffset)
{
	std::wstring targetName{ L"Skybox Texture" };
	DXHelper::CreateFenceSet(m_device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

	// Create Render Target View (RTV) heap for each face
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_skyboxVertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(glm::vec3)), static_cast<UINT>(sizeof(glm::vec3) * m_skyboxVertices.size()), m_skyboxVertices.data());
	std::vector<VA> vas;
	for (int i = 0; i < 4; ++i)
	{
		vas.push_back({ m_fullscreenQuad[i], m_fullscreenQuadTexCoords[i] });
	}
	m_quadVertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(VA)), static_cast<UINT>(sizeof(VA) * vas.size()), vas.data());
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

void DXSkybox::Initialize(const std::filesystem::path& path)
{
	m_equirectangularMap = std::make_unique<DXTexture>();
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	// Store equirectangular texture in srvHeap index 0
	m_equirectangularMap->LoadTexture(m_device, m_commandList, m_srvHeap, m_commandQueue, m_fence, m_fenceEvent, static_cast<UINT>(m_srvHeapStartOffset), true, path, "equirectangular", true);
	faceSize = m_equirectangularMap->GetHeight();

	// @TODO Fix Exception thrown when running through RenderDoc or on iGPU
	EquirectangularToCube();
	CalculateIrradiance();
	PrefilteredEnvironmentMap();
	BRDFLUT();
}

void DXSkybox::ExecuteCommandList()
{
	DXHelper::ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Wait for GPU
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
	DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
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

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = texDesc.Format;
	memcpy(clearValue.Color, clearColor, sizeof(float) * 4);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
#if USE_NSIGHT_AFTERMATH
	std::string eventMarker = "EquirectangularToCube()";
	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(m_hAftermathCommandListContext, (void*)eventMarker.c_str(), (unsigned int)eventMarker.size() + 1));
#endif
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_cubemap)
	));
	DXHelper::ThrowIfFailed(m_cubemap->SetName(L"Skybox Cubemap Resource"));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT f = 0; f < 6; ++f)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = f;
		rtvDesc.Texture2DArray.ArraySize = 1;
		m_device->CreateRenderTargetView(m_cubemap.Get(), &rtvDesc, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
	}

	// Prepare Pipeline
	CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 1;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[0])));
	DXHelper::ThrowIfFailed(m_rootSignatures[0]->SetName(L"Skybox Equirectangular Root Signature"));

	// Create Pipeline State Object (PSO)
	struct VA
	{
		glm::vec3 position;
	};

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VA, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	m_pipelines[0] = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignatures[0],
		"../Engine/shaders/hlsl/Cubemap.vert.hlsl",
		"../Engine/shaders/hlsl/Equirectangular.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{ positionLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		false,
		false,
		texDesc.Format
	);

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[0]->GetPipelineState().Get()));

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(faceSize), static_cast<FLOAT>(faceSize), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(faceSize), static_cast<LONG>(faceSize) };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignatures[0].Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE equirectangularSrvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	equirectangularSrvHandle.Offset(static_cast<INT>(m_srvHeapStartOffset), m_srvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, equirectangularSrvHandle);

	//DXConstantBuffer<WorldToNDC> matrixConstantBuffer(m_device, 1);

	D3D12_VERTEX_BUFFER_VIEW vbv = m_skyboxVertexBuffer->GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbv);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_cubemap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int f = 0; f < 6; ++f)
	{
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		WorldToNDC worldToNDC = { views[f], projection };
		//matrixConstantBuffer.UpdateConstant(&worldToNDC, 0);
		//m_commandList->SetGraphicsRootConstantBufferView(0, matrixConstantBuffer.GetGPUVirtualAddress(0));
		m_commandList->SetGraphicsRoot32BitConstants(0, 32, &worldToNDC, 0);

		m_commandList->DrawInstanced(36, 1, 0, 0);

		rtvHandle.ptr += m_rtvDescriptorSize;
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_cubemap.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	ExecuteCommandList();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;

	// Store Cubemap texture in srvHeap index 1
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<UINT>(m_srvHeapStartOffset) + 1, m_srvDescriptorSize);
	m_device->CreateShaderResourceView(m_cubemap.Get(), &srvDesc, srvHandle);
}

void DXSkybox::CalculateIrradiance()
{
	// Create Cubemap Resource
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT64>(irradianceSize);
	texDesc.Height = static_cast<UINT>(irradianceSize);
	texDesc.DepthOrArraySize = 6;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = texDesc.Format;
	memcpy(clearValue.Color, clearColor, sizeof(float) * 4);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
#if USE_NSIGHT_AFTERMATH
	std::string eventMarker = "CalculateIrradiance()";
	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(m_hAftermathCommandListContext, (void*)eventMarker.c_str(), (unsigned int)eventMarker.size() + 1));
#endif
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_irradianceMap)
	));
	DXHelper::ThrowIfFailed(m_irradianceMap->SetName(L"Skybox Irradiance Map Resource"));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT f = 0; f < 6; ++f)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = f;
		rtvDesc.Texture2DArray.ArraySize = 1;
		m_device->CreateRenderTargetView(m_irradianceMap.Get(), &rtvDesc, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
	}

	// Prepare Pipeline
	CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 1;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[1])));
	DXHelper::ThrowIfFailed(m_rootSignatures[1]->SetName(L"Skybox Irradiance Root Signature"));

	// Create Pipeline State Object (PSO)
	struct VA
	{
		glm::vec3 position;
	};

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VA, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	m_pipelines[1] = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignatures[1],
		"../Engine/shaders/hlsl/Cubemap.vert.hlsl",
		"../Engine/shaders/hlsl/Irradiance.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{ positionLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		false,
		false,
		texDesc.Format
	);

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[1]->GetPipelineState().Get()));

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(irradianceSize), static_cast<FLOAT>(irradianceSize), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(irradianceSize), static_cast<LONG>(irradianceSize) };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignatures[1].Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cubemapSrvHandle{ m_srvHeap->GetGPUDescriptorHandleForHeapStart() };
	cubemapSrvHandle.Offset(static_cast<INT>(m_srvHeapStartOffset) + 1, m_srvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, cubemapSrvHandle);

	//DXConstantBuffer<WorldToNDC> matrixConstantBuffer(m_device, 1);

	D3D12_VERTEX_BUFFER_VIEW vbv = m_skyboxVertexBuffer->GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbv);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_irradianceMap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int f = 0; f < 6; ++f)
	{
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		WorldToNDC worldToNDC = { views[f], projection };
		//matrixConstantBuffer.UpdateConstant(&worldToNDC, 0);
		//m_commandList->SetGraphicsRootConstantBufferView(0, matrixConstantBuffer.GetGPUVirtualAddress(0));
		m_commandList->SetGraphicsRoot32BitConstants(0, 32, &worldToNDC, 0);

		m_commandList->DrawInstanced(36, 1, 0, 0);

		rtvHandle.ptr += m_rtvDescriptorSize;
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_irradianceMap.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	ExecuteCommandList();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;

	// Store Irradiance texture in srvHeap index 2
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<UINT>(m_srvHeapStartOffset) + 2, m_srvDescriptorSize);
	m_device->CreateShaderResourceView(m_irradianceMap.Get(), &srvDesc, srvHandle);
}

void DXSkybox::PrefilteredEnvironmentMap()
{
	// Create Cubemap Resource
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT64>(baseSize);
	texDesc.Height = static_cast<UINT>(baseSize);
	texDesc.DepthOrArraySize = 6;
	texDesc.MipLevels = mipLevels;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = texDesc.Format;
	memcpy(clearValue.Color, clearColor, sizeof(float) * 4);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
#if USE_NSIGHT_AFTERMATH
	std::string eventMarker = "PrefilterEnvironmentMap()";
	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(m_hAftermathCommandListContext, (void*)eventMarker.c_str(), (unsigned int)eventMarker.size() + 1));
#endif
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_prefilterMap)
	));
	DXHelper::ThrowIfFailed(m_prefilterMap->SetName(L"Skybox Prefilter Map Resource"));

	// Prepare Pipeline
	CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 1;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[2])));
	DXHelper::ThrowIfFailed(m_rootSignatures[2]->SetName(L"Skybox Prefilter Root Signature"));

	// Create Pipeline State Object (PSO)
	struct VA
	{
		glm::vec3 position;
	};

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VA, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	m_pipelines[2] = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignatures[2],
		"../Engine/shaders/hlsl/Cubemap.vert.hlsl",
		"../Engine/shaders/hlsl/Prefilter.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{ positionLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		false,
		false,
		texDesc.Format
	);

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[2]->GetPipelineState().Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignatures[2].Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cubemapSrvHandle{ m_srvHeap->GetGPUDescriptorHandleForHeapStart() };
	cubemapSrvHandle.Offset(static_cast<INT>(m_srvHeapStartOffset) + 1, m_srvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, cubemapSrvHandle);

	DXConstantBuffer<float>roughnessConstantBuffer(m_device, 1);

	D3D12_VERTEX_BUFFER_VIEW vbv = m_skyboxVertexBuffer->GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbv);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_prefilterMap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	for (int mip = 0; mip < mipLevels; ++mip)
	{
		uint32_t dim = baseSize >> mip;
		float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);

		D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(dim), static_cast<FLOAT>(dim), 0.f, 1.f };
		D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(dim), static_cast<LONG>(dim) };
		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &scissorRect);

		//roughnessConstantBuffer.UpdateConstant(&roughness, 0);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };
		for (int f = 0; f < 6; ++f)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = texDesc.Format;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = mip;
			rtvDesc.Texture2DArray.FirstArraySlice = f;
			rtvDesc.Texture2DArray.ArraySize = 1;

			CD3DX12_CPU_DESCRIPTOR_HANDLE currentRtvHandle(rtvHandle, f, m_rtvDescriptorSize);
			m_device->CreateRenderTargetView(m_prefilterMap.Get(), &rtvDesc, currentRtvHandle);

			m_commandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, nullptr);
			m_commandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

			WorldToNDC worldToNDC = { views[f], projection };
			//matrixConstantBuffer.UpdateConstant(&worldToNDC, 0);
			//m_commandList->SetGraphicsRootConstantBufferView(0, matrixConstantBuffer.GetGPUVirtualAddress(0));
			m_commandList->SetGraphicsRoot32BitConstants(0, 32, &worldToNDC, 0);
			m_commandList->SetGraphicsRoot32BitConstants(2, 1, &roughness, 0);

			m_commandList->DrawInstanced(36, 1, 0, 0);
		}
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_prefilterMap.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	ExecuteCommandList();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = mipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	// Store Prefilter texture in srvHeap index 3
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<UINT>(m_srvHeapStartOffset) + 3, m_srvDescriptorSize);
	m_device->CreateShaderResourceView(m_prefilterMap.Get(), &srvDesc, srvHandle);
}

void DXSkybox::BRDFLUT()
{
	// Create Cubemap Resource
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT64>(lutSize);
	texDesc.Height = static_cast<UINT>(lutSize);
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = texDesc.Format;
	memcpy(clearValue.Color, clearColor, sizeof(float) * 4);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
#if USE_NSIGHT_AFTERMATH
	std::string eventMarker = "BRDFLUT()";
	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(m_hAftermathCommandListContext, (void*)eventMarker.c_str(), (unsigned int)eventMarker.size() + 1));
#endif
	DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_brdfLUT)
	));
	DXHelper::ThrowIfFailed(m_brdfLUT->SetName(L"Skybox BRDF LUT Resource"));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	m_device->CreateRenderTargetView(m_brdfLUT.Get(), &rtvDesc, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	// Prepare Pipeline
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[3])));
	DXHelper::ThrowIfFailed(m_rootSignatures[3]->SetName(L"Skybox BRDF LUT Root Signature"));

	// Create Pipeline State Object (PSO)
	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VA, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout uvLayout{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VA, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	m_pipelines[3] = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignatures[3],
		"../Engine/shaders/hlsl/BRDF.vert.hlsl",
		"../Engine/shaders/hlsl/BRDF.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{ positionLayout, uvLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		false,
		false,
		texDesc.Format
	);

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[3]->GetPipelineState().Get()));

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(lutSize), static_cast<FLOAT>(lutSize), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(lutSize), static_cast<LONG>(lutSize) };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignatures[3].Get());

	D3D12_VERTEX_BUFFER_VIEW vbv = m_quadVertexBuffer->GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbv);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_brdfLUT.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->DrawInstanced(4, 1, 0, 0);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_brdfLUT.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	ExecuteCommandList();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Store BRDF LUT texture in srvHeap index 4
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<UINT>(m_srvHeapStartOffset) + 4, m_srvDescriptorSize);
	m_device->CreateShaderResourceView(m_brdfLUT.Get(), &srvDesc, srvHandle);
}
