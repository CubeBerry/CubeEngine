//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.cpp
#include "Engine.hpp"

DXRenderManager::~DXRenderManager()
{
	void WaitForGPU();
	CloseHandle(m_fenceEvent);
}

void DXRenderManager::Initialize(SDL_Window* window_)
{
	// Get HWND (Handle to a Window)
	SDL_PropertiesID props = SDL_GetWindowProperties(window_);
	HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

	// Initialize Direct3D 12
	HRESULT hr;

	// Enable Debug Layer
#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			ComPtr<ID3D12Debug1> debugController1;
			if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1))))
			{
				debugController1->SetEnableGPUBasedValidation(TRUE);
			}
		}
	}
#endif

	// Create GXDI factory
	ComPtr<IDXGIFactory4> factory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create DXGI factory.");
	}

	// Create Device
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter);
	hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create device.");
	}

	// Create Command Queue
	// DESC = Description
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create command queue.");
	}

	// Create Swap Chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = frameCount; // Double buffering
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	hr = factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create swap chain.");
	}
	hr = swapChain.As(&m_swapChain);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to cast swap chain to IDXGISwapChain3.");
	}

	// Create Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameCount; // Double buffering
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create RTV descriptor heap.");
	}

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 500;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create SRV descriptor heap.");
	}

	// Create Render Target Views (RTVs)
	UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < frameCount; ++i)
	{
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to get swap chain buffer.");
		}
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}

	// Create Command Allocator
	for (UINT i = 0; i < frameCount; ++i)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create command allocator.");
		}
	}

	CreateRootSignature();

	DXAttributeLayout layout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(TwoDimension::Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	
	m_pipeline2D = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignature,
		std::filesystem::path("../Engine/shaders/hlsl/2D.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/2D.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{ layout },
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	// Create Command List
	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipeline2D->GetPipelineState().Get(), IID_PPV_ARGS(&m_commandList));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create command list.");
	}
	m_commandList->Close();

	// Create Fence for synchronization
	hr = m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create fence.");
	}
	// @TODO Why do m_fenceValues[m_frameIndex]++?
	m_fenceValues[m_frameIndex]++;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		throw std::runtime_error("Failed to create fence event.");
	}

	WaitForGPU();

	SDL_ShowWindow(window_);
}

bool DXRenderManager::BeginRender(glm::vec3 bgColor)
{
	m_commandAllocators[m_frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Set the viewport and scissor rect
	int width, height;
	SDL_GetWindowSize(Engine::GetWindow().GetWindow(), &width, &height);
	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, width, height };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_frameIndex), m_rtvDescriptorSize);
	// @TODO What does OMSetRenderTargets do?
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { bgColor.r, bgColor.g, bgColor.b, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<Sprite*> sprites = Engine::Instance().GetSpriteManager().GetSprites();
	for (const auto& sprite : sprites)
	{
		auto& buffer = sprite->GetBufferWrapper()->GetBuffer<BufferWrapper::DXBuffer>();
		auto& constantBuffer = sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>();

		// Update Constant Buffer
		constantBuffer.vertexUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform, m_frameIndex);
		constantBuffer.fragmentUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform, m_frameIndex);

		// Bind Vertex Buffer & Index Buffer
		D3D12_VERTEX_BUFFER_VIEW vbv = buffer.vertexBuffer->GetView();
		D3D12_INDEX_BUFFER_VIEW ibv = buffer.indexBuffer->GetView();
		m_commandList->IASetVertexBuffers(0, 1, &vbv);
		m_commandList->IASetIndexBuffer(&ibv);
		// Bind constant buffers to root signature
		m_commandList->SetGraphicsRootConstantBufferView(0, constantBuffer.vertexUniformBuffer->GetConstantBuffer()->GetGPUVirtualAddress());
		m_commandList->SetGraphicsRootConstantBufferView(1, constantBuffer.fragmentUniformBuffer->GetConstantBuffer()->GetGPUVirtualAddress());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->DrawIndexedInstanced(static_cast<UINT>(sprite->GetBufferWrapper()->GetIndices().size()), 1, 0, 0, 0);
	}

	return true;
}

void DXRenderManager::EndRender()
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();

	// Execute the command list
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_swapChain->Present(1, 0);

	// Wait for the GPU to finish
	WaitForGPU();

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DXRenderManager::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter)
{
	*ppAdapter = nullptr;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (UINT i = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); ++i)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))) break;
		}
	}
	if (adapter.Get() == nullptr)
	{
		for (UINT i = 0; SUCCEEDED(pFactory->EnumAdapters1(i, &adapter)); ++i)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))) break;
		}
	}
	*ppAdapter = adapter.Detach();
}

void DXRenderManager::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 srvRange;
	srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 500, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	// The slot of a root signature version 1.1
	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 1;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// @TODO What is root signature?
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(
		_countof(rootParameters), rootParameters,
		1, &sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signature, error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	if (FAILED(hr))
	{
		if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
		throw std::runtime_error("Failed to serialize root signature.");
	}

	hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create root signature.");
	}
}

void DXRenderManager::WaitForGPU()
{
	// Is this a good way to synchronize? -> https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloWindow/D3D12HelloWindow.cpp
	if (SUCCEEDED(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex])))
	{
		if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
		{
			if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent)))
			{
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
		}
		m_fenceValues[m_frameIndex]++;
	}
}

void DXRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
	DXTexture* texture = new DXTexture();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
	texture->LoadTexture(
		m_device,
		m_commandList,
		m_srvHeap,
		m_commandQueue,
		m_fence,
		m_fenceEvent,
		false,
		path_,
		name_,
		flip
	);

	WaitForGPU();

	textures.push_back(texture);

	int texId = static_cast<int>(textures.size() - 1);
	textures.at(texId)->SetTextureID(texId);
}

void DXRenderManager::DeleteWithIndex(int /*id*/)
{
	//Destroy Texture
	for (auto t : textures)
		delete t;
	textures.erase(textures.begin(), textures.end());
}

//DXTexture* DXRenderManager::GetTexture(std::string name)
//{
//	for (auto& tex : textures)
//	{
//		if (tex->GetName() == name)
//		{
//			return tex;
//		}
//	}
//	return nullptr;
//}

void DXRenderManager::LoadSkybox(const std::filesystem::path& path)
{
	path;
	//skyboxVertexArray.Initialize();

	//float skyboxVertices[] = {
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	-1.0f,  1.0f, -1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	 1.0f, -1.0f,  1.0f
	//};

	//skyboxVertexBuffer = new DXVertexBuffer;
	//skyboxVertexBuffer->SetData(sizeof(float) * 108, skyboxVertices);

	//DXAttributeLayout position_layout;
	//position_layout.component_type = DXAttributeLayout::Float;
	//position_layout.component_dimension = DXAttributeLayout::_3;
	//position_layout.normalized = false;
	//position_layout.vertex_layout_location = 0;
	//position_layout.stride = sizeof(float) * 3;
	//position_layout.offset = 0;
	//position_layout.relative_offset = 0;

	//skyboxVertexArray.AddVertexBuffer(std::move(*skyboxVertexBuffer), sizeof(float) * 3, { position_layout });

	//skyboxShader.LoadShader({ { DXShader::VERTEX, "../Engine/shaders/glsl/Skybox.vert" }, { DXShader::FRAGMENT, "../Engine/shaders/glsl/Skybox.frag" } });
	//skybox = new DXSkybox(path);

	////Revert DX_TEXTURE0 which is binded(covered) by BRDFLUT's texture when loading skybox to first loaded texture
	//if (!textures.empty())
	//{
	//	glActiveTexture(DX_TEXTURE0);
	//	glBindTexture(DX_TEXTURE_2D, textures[0]->GetTextureHandle());
	//}

	//skyboxEnabled = true;
}

void DXRenderManager::DeleteSkybox()
{
	//delete skyboxVertexBuffer;
	//delete skybox;
	//skyboxEnabled = false;
}
