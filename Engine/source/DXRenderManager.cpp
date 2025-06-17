//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.cpp
#include "DXRenderManager.hpp"

#include "Engine.hpp"

DXRenderManager::~DXRenderManager()
{
	WaitForGPU();
	CloseHandle(m_fenceEvent);

	//Destroy Buffers
	delete directionalLightUniformBuffer;
	delete pointLightUniformBuffer;
}

void DXRenderManager::Initialize(SDL_Window* window)
{
	// Get HWND (Handle to a Window)
	SDL_PropertiesID props = SDL_GetWindowProperties(window);
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
	m_device->SetName(L"Main Device");

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
	m_commandQueue->SetName(L"Main Command Queue");

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
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create Descriptor Heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameCount; // Double buffering
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create RTV descriptor heap.");
	}
	m_rtvHeap->SetName(L"Render Target View Heap");

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create DSV descriptor heap.");
	}
	m_dsvHeap->SetName(L"Depth/Stencil View Heap");

	// Create Depth Stencil Buffer Resource
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	depthStencilDesc.Width = static_cast<UINT64>(width);
	depthStencilDesc.Height = static_cast<UINT>(height);
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	// @TODO Use ID3D12Device::CheckFeatureSupport to find supported format
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthStencil)
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create depth stencil resource.");
	}
	m_depthStencil->SetName(L"Depth/Stencil Resource");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 500;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create SRV descriptor heap.");
	}
	m_srvHeap->SetName(L"Shader Resource View Heap");

	// Create Render Target Views (RTVs)
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < frameCount; ++i)
	{
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to get swap chain buffer.");
		}
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
		m_renderTargets[i]->SetName((L"Render Target " + std::to_wstring(i)).c_str());
	}

	// Create Command Allocator
	for (UINT i = 0; i < frameCount; ++i)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create command allocator.");
		}
		m_commandAllocators[i]->SetName((L"Command Allocator " + std::to_wstring(i)).c_str());
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
	m_fence->SetName(L"Main Fence");

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		throw std::runtime_error("Failed to create fence event.");
	}

	m_imguiManager = std::make_unique<DXImGuiManager>(
		Engine::GetWindow().GetWindow(), m_device, frameCount
	);

	SDL_ShowWindow(window);
}

bool DXRenderManager::BeginRender(glm::vec3 bgColor)
{
	m_commandAllocators[m_frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

	m_commandList->SetPipelineState(m_pipeline2D->GetPipelineState().Get());

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

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
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	// @TODO What does OMSetRenderTargets do?
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	const float clearColor[] = { bgColor.r, bgColor.g, bgColor.b, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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
		m_commandList->SetGraphicsRootConstantBufferView(0, constantBuffer.vertexUniformBuffer->GetGPUVirtualAddress(m_frameIndex));
		m_commandList->SetGraphicsRootConstantBufferView(1, constantBuffer.fragmentUniformBuffer->GetGPUVirtualAddress(m_frameIndex));

		m_commandList->DrawIndexedInstanced(static_cast<UINT>(sprite->GetBufferWrapper()->GetIndices().size()), 1, 0, 0, 0);
	}

	m_imguiManager->Begin();

	return true;
}

void DXRenderManager::EndRender()
{
	m_imguiManager->End(m_commandList);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();

	// Execute the command list
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	HRESULT hr = m_swapChain->Present(1, 0);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to present swap chain.");
	}

	MoveToNextFrame();
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
	srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 500, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

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
	m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]);
	HRESULT hr = m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to set event on fence completion.");
	}

	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValues[m_frameIndex]++;
}

void DXRenderManager::MoveToNextFrame()
{
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		HRESULT hr = m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to set event on fence completion.");
		}
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void DXRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
	// Create Command Allocator
	ComPtr<ID3D12CommandAllocator> tempCommandAllocator;
	HRESULT hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tempCommandAllocator));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create texture command allocator.");
	}
	tempCommandAllocator->SetName(L"Texture Command Allocator");

	// Create Command List
	ComPtr<ID3D12GraphicsCommandList> tempCommandList;
	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, tempCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&tempCommandList));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create texture command list.");
	}
	tempCommandList->SetName(L"Texture Command List");

	// Create Fence
	ComPtr<ID3D12Fence> tempFence;
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tempFence));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create texture fence.");
	}
	tempFence->SetName(L"Texture Fence");

	// Create Fence Event
	HANDLE tempFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (tempFenceEvent == nullptr)
	{
		throw std::runtime_error("Failed to create texture fence event.");
	}

	const auto& texture = textures.emplace_back(std::make_unique<DXTexture>());

	const int texId = static_cast<int>(textures.size() - 1);
	texture->SetTextureID(texId);

	texture->LoadTexture(
		m_device,
		tempCommandList,
		m_srvHeap,
		m_commandQueue,
		tempFence,
		tempFenceEvent,
		false,
		path_,
		name_,
		flip
	);
}

void DXRenderManager::ClearTextures()
{
	textures.clear();
}

DXTexture* DXRenderManager::GetTexture(const std::string& name) const
{
	for (auto& tex : textures)
	{
		if (tex->GetName() == name)
		{
			return tex.get();
		}
	}
	return nullptr;
}

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
