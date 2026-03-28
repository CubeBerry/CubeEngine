//Author: JEYOON YU
//Project: CubeEngine
//File: DXSkybox.cpp
#include "DXSkybox.hpp"
#include <filesystem>

#include "DXHelper.hpp"
#include "DXInitializer.hpp"
#include "DXTexture.hpp"
#include "DXConstantBuffer.hpp"

DXSkybox::DXSkybox(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12CommandQueue>& commandQueue,
	const ComPtr<ID3D12DescriptorHeap>& srvHeap) : m_device(device), m_commandQueue(commandQueue), m_srvHeap(srvHeap)
{
	std::wstring targetName{ L"Skybox Texture" };
	DXInitializer::CreateFenceSet(m_device, targetName, m_commandAllocator, m_commandList, m_fence, m_fenceEvent);

	// Create Render Target View (RTV) heap for each face
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_skyboxVertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(glm::vec3)), static_cast<UINT>(sizeof(glm::vec3) * m_skyboxVertices.size()), m_skyboxVertices.data());

	m_mipmapGenerator = std::make_unique<DXMipmapGenerator>(m_device);
}

DXSkybox::~DXSkybox()
{
	if (m_deallocator)
	{
		// index 0 will deallocate when m_equirectangularMap is deleted
		// Deallocate Cubemap, Irradiance, Prefilter, BRDF LUT
		for (int i = 1; i < 5; ++i)
		{
			m_deallocator(m_srvHandles[i].second);
		}
	}

	//m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
	//HRESULT hr = m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	//if (FAILED(hr))
	//{
	//	throw std::runtime_error("Failed to set event on fence completion.");
	//}
	//WaitForSingleObject(m_fenceEvent, INFINITE);
	//CloseHandle(m_fenceEvent);
}

void DXSkybox::Initialize(const std::filesystem::path& path,
	const std::array<std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT>, 5>& srvHandles,
	std::function<void(UINT)> deallocator)
{
	m_srvHandles = srvHandles;
	m_deallocator = std::move(deallocator);

	m_equirectangularMap = std::make_unique<DXTexture>();
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	// Store equirectangular texture in srvHeap index 0
	m_equirectangularMap->LoadTexture(m_device, m_commandList, m_commandQueue, srvHandles[0], m_deallocator, m_fence, m_fenceEvent, true, path, "Equirectangular", true);
	faceSize = m_equirectangularMap->GetHeight();

	EquirectangularToCube();
	//CalculateIrradiance();
	// Spherical Harmonics based irradiance calculation
	// Should free texture data after calculating SH coefficients
	std::vector<glm::vec3> E_lm = CalculateSHCoefficients(static_cast<float*>(m_equirectangularMap->GetTextureData()), m_equirectangularMap->GetWidth(), m_equirectangularMap->GetHeight());
	CalculateIrradiance(E_lm);
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
	UINT16 maxMipLevels = static_cast<UINT16>(std::log2(faceSize)) + 1;
	texDesc.MipLevels = maxMipLevels;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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

	std::vector<DXGI_FORMAT> rtvFormats = { texDesc.Format };
	m_pipelines[0] = DXPipeLineBuilder(m_device, m_rootSignatures[0])
		.SetShaders("../Engine/shaders/hlsl/Cubemap.vert.hlsl", "../Engine/shaders/hlsl/Equirectangular.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.Build();

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
	equirectangularSrvHandle.Offset(m_srvHandles[0].second, m_srvDescriptorSize);
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

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_cubemap.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	m_mipmapGenerator->Generate(m_device, m_commandList, m_cubemap.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_cubemap.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	ExecuteCommandList();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.TextureCube.MipLevels = maxMipLevels;

	// Store Cubemap texture in second array of m_srvDescriptorIndices offset
	m_device->CreateShaderResourceView(m_cubemap.Get(), &srvDesc, m_srvHandles[1].first);
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

	std::vector<DXGI_FORMAT> rtvFormats = { texDesc.Format };
	m_pipelines[1] = DXPipeLineBuilder(m_device, m_rootSignatures[1])
		.SetShaders("../Engine/shaders/hlsl/Cubemap.vert.hlsl", "../Engine/shaders/hlsl/Irradiance.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.Build();

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
	cubemapSrvHandle.Offset(m_srvHandles[1].second, m_srvDescriptorSize);
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
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.TextureCube.MipLevels = 1;

	// Store Irradiance texture in third array of m_srvDescriptorIndices offset
	m_device->CreateShaderResourceView(m_irradianceMap.Get(), &srvDesc, m_srvHandles[2].first);
}

std::vector<glm::vec3> DXSkybox::CalculateSHCoefficients(const float* hdrData, int width, int height)
{
	std::vector<glm::vec3> L_lm(9, glm::vec3(0.0f));

	float PI = glm::pi<float>();
	float dTheta = PI / static_cast<float>(height);
	float dPhi = 2.0f * PI / static_cast<float>(width);

	for (int j = 0; j < height; ++j)
	{
		float theta = PI * (static_cast<float>(j) + 0.5f) / static_cast<float>(height);
		float sinTheta = std::sin(theta);
		float cosTheta = std::cos(theta);

		for (int i = 0; i < width; ++i)
		{
			float phi = 2.0f * PI * (static_cast<float>(i) + 0.5f) / static_cast<float>(width);
			float sinPhi = std::sin(phi);
			float cosPhi = std::cos(phi);

			// Y-axis points up, Z-axis points forward, X-axis points right
			// Convert from spherical coordinates to Cartesian coordinates
			// Minus sign is needed to flip the direction of the vector to match the cubemap's coordinate system
			float x = -sinTheta * cosPhi;
			float y = -cosTheta;
			float z = -sinTheta * sinPhi;

			int index = (j * width + i) * 4;
			glm::vec3 color(hdrData[index], hdrData[index + 1], hdrData[index + 2]);

			float dArea = sinTheta * dTheta * dPhi;

			float Y00 = 0.5f * std::sqrt(1.0f / PI);

			float Y1_1 = 0.5f * std::sqrt(3.0f / PI) * y;
			float Y10 = 0.5f * std::sqrt(3.0f / PI) * z;
			float Y11 = 0.5f * std::sqrt(3.0f / PI) * x;

			float Y2_2 = 0.5f * std::sqrt(15.0f / PI) * x * y;
			float Y2_1 = 0.5f * std::sqrt(15.0f / PI) * y * z;
			float Y20 = 0.25f * std::sqrt(5.0f / PI) * (3.0f * z * z - 1.0f);
			float Y21 = 0.5f * std::sqrt(15.0f / PI) * x * z;
			float Y22 = 0.25f * std::sqrt(15.0f / PI) * (x * x - y * y);

			L_lm[0] += color * Y00 * dArea;
			L_lm[1] += color * Y1_1 * dArea;
			L_lm[2] += color * Y10 * dArea;
			L_lm[3] += color * Y11 * dArea;
			L_lm[4] += color * Y2_2 * dArea;
			L_lm[5] += color * Y2_1 * dArea;
			L_lm[6] += color * Y20 * dArea;
			L_lm[7] += color * Y21 * dArea;
			L_lm[8] += color * Y22 * dArea;
		}
	}

	float A0 = PI;
	float A1 = 2.0f / 3.0f * PI;
	float A2 = 0.25f * PI;

	std::vector<glm::vec3> E_lm(9);
	E_lm[0] = L_lm[0] * A0;

	E_lm[1] = L_lm[1] * A1;
	E_lm[2] = L_lm[2] * A1;
	E_lm[3] = L_lm[3] * A1;

	E_lm[4] = L_lm[4] * A2;
	E_lm[5] = L_lm[5] * A2;
	E_lm[6] = L_lm[6] * A2;
	E_lm[7] = L_lm[7] * A2;
	E_lm[8] = L_lm[8] * A2;

	return E_lm;
}

// Spherical Harmonics based irradiance calculation
void DXSkybox::CalculateIrradiance(const std::vector<glm::vec3>& E_lm)
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
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[1])));
	DXHelper::ThrowIfFailed(m_rootSignatures[1]->SetName(L"Skybox Spherical Harmonics Irradiance Root Signature"));

	// Create Pipeline State Object (PSO)
	struct VA
	{
		glm::vec3 position;
	};

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VA, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	std::vector<DXGI_FORMAT> rtvFormats = { texDesc.Format };
	m_pipelines[1] = DXPipeLineBuilder(m_device, m_rootSignatures[1])
		.SetShaders("../Engine/shaders/hlsl/Cubemap.vert.hlsl", "../Engine/shaders/hlsl/SphericalHarmonicsIrradiance.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.Build();

	SHCoefficients shData;
	for (int i = 0; i < 9; ++i)
	{
		shData.E_lm[i] = glm::vec4(E_lm[i], 0.0f);
	}
	DXConstantBuffer<SHCoefficients> shConstantBuffer(m_device, 1);
	shConstantBuffer.UpdateConstant(&shData, sizeof(SHCoefficients), 0);

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[1]->GetPipelineState().Get()));

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(irradianceSize), static_cast<FLOAT>(irradianceSize), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(irradianceSize), static_cast<LONG>(irradianceSize) };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	m_commandList->SetGraphicsRootSignature(m_rootSignatures[1].Get());

	m_commandList->SetGraphicsRootConstantBufferView(1, shConstantBuffer.GetGPUVirtualAddress(0));

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
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.TextureCube.MipLevels = 1;

	// Store Irradiance texture in third array of m_srvDescriptorIndices offset
	m_device->CreateShaderResourceView(m_irradianceMap.Get(), &srvDesc, m_srvHandles[2].first);
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
	// MinLOD and MaxLOD are set to allow sampling across all mip levels of the cubemap, which is essential for accurate prefiltering results
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
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

	std::vector<DXGI_FORMAT> rtvFormats = { texDesc.Format };
	m_pipelines[2] = DXPipeLineBuilder(m_device, m_rootSignatures[2])
		.SetShaders("../Engine/shaders/hlsl/Cubemap.vert.hlsl", "../Engine/shaders/hlsl/Prefilter.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.Build();

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[2]->GetPipelineState().Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignatures[2].Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cubemapSrvHandle{ m_srvHeap->GetGPUDescriptorHandleForHeapStart() };
	cubemapSrvHandle.Offset(m_srvHandles[1].second, m_srvDescriptorSize);
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
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = mipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	// Store Prefilter texture in fourth array of m_srvDescriptorIndices offset
	m_device->CreateShaderResourceView(m_prefilterMap.Get(), &srvDesc, m_srvHandles[3].first);
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
	std::vector<DXGI_FORMAT> rtvFormats = { texDesc.Format };
	m_pipelines[3] = DXPipeLineBuilder(m_device, m_rootSignatures[3])
		.SetShaders("../Engine/shaders/hlsl/BRDF.vert.hlsl", "../Engine/shaders/hlsl/BRDF.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{})
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.Build();

	// Record Command List
	DXHelper::ThrowIfFailed(m_commandAllocator->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelines[3]->GetPipelineState().Get()));

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(lutSize), static_cast<FLOAT>(lutSize), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(lutSize), static_cast<LONG>(lutSize) };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignatures[3].Get());

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
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Store BRDF LUT texture in fifth array of m_srvDescriptorIndices offset
	m_device->CreateShaderResourceView(m_brdfLUT.Get(), &srvDesc, m_srvHandles[4].first);
}
