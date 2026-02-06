//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.cpp
#include "DXRenderManager.hpp"

#include <d3d12.h>

#include "DXCommandListWrapper.hpp"
#include "DXHelper.hpp"
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

	// Enable Debug Layer
	// The D3D debug layer (as well as Microsoft PIX and other graphics debugger
	// tools using an injection library) is not compatible with Nsight Aftermath!
	// If Aftermath detects that any of these tools are present it will fail
	// initialization.
#if defined(_DEBUG) && !USE_NSIGHT_AFTERMATH
	{
		ComPtr<ID3D12Debug> debugController;
		DXHelper::ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
		ComPtr<ID3D12Debug1> debugController1;
		DXHelper::ThrowIfFailed(debugController->QueryInterface(IID_PPV_ARGS(&debugController1)));
		debugController1->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// Create GXDI factory
	ComPtr<IDXGIFactory4> factory;
	DXHelper::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

#if USE_NSIGHT_AFTERMATH
	m_gpuCrashTracker.Initialize();
#endif

#if USE_PREVIEW_SDK
	// Enable Experimental Features for Mesh Nodes
	// Windows Developer Mode must be enabled to use Mesh Nodes
	UUID features[2] = { D3D12ExperimentalShaderModels, D3D12StateObjectsExperiment };
	DXHelper::ThrowIfFailed(D3D12EnableExperimentalFeatures(_countof(features), features, nullptr, nullptr));
#endif

	// Create Device
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter);
	DXHelper::ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
	DXHelper::ThrowIfFailed(m_device->SetName(L"Main Device"));

	// Check Mesh Shader Support
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 options{};
	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options, sizeof(options)))
		|| (options.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
	{
		m_meshShaderEnabled = false;
		Engine::GetLogger().LogDebug(LogCategory::D3D12, "Mesh Shader Not Supported");
		OutputDebugStringA("Mesh Shaders Not Supported\n");
	}
	else
	{
		// Check Shader Model 6.5 Support for Mesh Shader
		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
			|| (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
		{
			m_meshShaderEnabled = false;
			Engine::GetLogger().LogDebug(LogCategory::D3D12, "Mesh Shader, Shader Model 6.5 Not Supported");
			OutputDebugStringA("Mesh Shader, Shader Model 6.5 Not Supported\n");
		}
		else
		{
			m_meshShaderEnabled = true;
			Engine::GetLogger().LogDebug(LogCategory::D3D12, "Mesh Shader Enabled");
			OutputDebugStringA("Mesh Shader Enabled\n");
		}
	}

	// Initialize Work Graphs Context
	m_workGraphsContext = std::make_unique<DXWorkGraphsContext>(this);
	// Check Work Graphs Support
	m_workGraphsContext->CheckWorkGraphsSupport();
#if USE_PREVIEW_SDK
	// Check Mesh Nodes Support
	m_workGraphsContext->CheckMeshNodesSupport();
#endif

#if USE_NSIGHT_AFTERMATH
	const uint32_t aftermathFlags =
		GFSDK_Aftermath_FeatureFlags_EnableMarkers |             // Enable event marker tracking.
		GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |    // Enable tracking of resources.
		GFSDK_Aftermath_FeatureFlags_CallStackCapturing |        // Capture call stacks for all draw calls, compute dispatches, and resource copies.
		GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo;    // Generate debug information for shaders.

	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_Initialize(
		GFSDK_Aftermath_Version_API,
		aftermathFlags,
		m_device.Get()));
#endif

	// Create Command Queue
	// DESC = Descriptor
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	DXHelper::ThrowIfFailed(m_commandQueue->SetName(L"Main Command Queue"));

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
	DXHelper::ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));
	DXHelper::ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create Descriptor Heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameCount; // Double buffering
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	DXHelper::ThrowIfFailed(m_rtvHeap->SetName(L"Render Target View Heap"));

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	// ----------------------------------------------------------------------------------
	// SRV Descriptor Heap Partitioning Strategy (Total: 1007)
	// 
	// [Range 0 ~ 499] : Texture Pool (MAX_OBJECT_SIZE)
	//    - Reserved exclusively for 2D Sprite Textures.
	//    - Managed by: m_textureDescriptorOffset
	// 
	// [Range 500 ~ 1006] : General Resource Pool
	//    - Used for Meshlet Buffers, Skybox, Work Graphs, Compute Shaders, etc.
	//    - Managed by: m_srvDescriptorOffset (Starts at 500)
	// ----------------------------------------------------------------------------------
	srvHeapDesc.NumDescriptors = 1007;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	DXHelper::ThrowIfFailed(m_srvHeap->SetName(L"Shader Resource View Heap"));
	m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_textureDescriptorOffset = 0;
	m_srvDescriptorOffset = MAX_OBJECT_SIZE;

	// Create Render Target Views (RTVs)
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < frameCount; ++i)
	{
		DXHelper::ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
		DXHelper::ThrowIfFailed(m_renderTargets[i]->SetName((L"Render Target " + std::to_wstring(i)).c_str()));
	}

	// Create Command Allocator
	for (UINT i = 0; i < frameCount; ++i)
	{
		DXHelper::ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
		DXHelper::ThrowIfFailed(m_commandAllocators[i]->SetName((L"Command Allocator " + std::to_wstring(i)).c_str()));
	}

	// Create Post-Process Context
	m_width = static_cast<int>(Engine::GetWindow().GetWindowSize().x);
	m_height = static_cast<int>(Engine::GetWindow().GetWindowSize().y);
	m_postProcessContext = std::make_unique<DXPostProcessContext>(this);
	m_postProcessContext->Initialize();

	// Create Render Target (HDR, MSAA, Depth)
	m_renderTarget = std::make_unique<DXRenderTarget>(
		m_device, window,
		m_postProcessContext->GetFidelityFX()->GetRenderWidth(),
		m_postProcessContext->GetFidelityFX()->GetRenderHeight(),
		m_deferredRenderingEnabled
	);
	// Allocate SRV handle for tone mapping
	// @TODO Find a way to handle SRV inside DXRenderTarget class
	m_hdrSrvHandle = AllocateSrvHandles();
	m_renderTarget->CreateSRV(m_hdrSrvHandle.first);

	// Create 2D Forward Render Context
	m_2dRenderContext = std::make_unique<DX2DRenderContext>(this);
	m_2dRenderContext->Initialize();

	// Create Forward Render Context
	m_forwardRenderContext = std::make_unique<DXForwardRenderContext>(this);
	m_forwardRenderContext->Initialize();

	// Create G-Buffer Context
	m_gBufferContext = std::make_unique<DXGBufferContext>(this);
	m_gBufferContext->Initialize();

	// Create Naive Render Context
	m_naiveLightingContext = std::make_unique<DXNaiveLightingContext>(this);
	m_naiveLightingContext->Initialize();

	// Create Global Lighting Context
	m_globalLightingContext = std::make_unique<DXGlobalLightingContext>(this);
	m_globalLightingContext->Initialize();

	// Create Local Lighting Context
	m_localLightingContext = std::make_unique<DXLocalLightingContext>(this);
	m_localLightingContext->Initialize();

	// Create Skybox Render Context
	m_skyboxRenderContext = std::make_unique<DXSkyboxRenderContext>(this);
	m_skyboxRenderContext->Initialize();

	// Create Command List
	DXHelper::ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	DXHelper::ThrowIfFailed(m_commandList->SetName(L"Main Command List"));
#if USE_NSIGHT_AFTERMATH
	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_CreateContextHandle(m_commandList.Get(), &m_hAftermathCommandListContext));
#endif
	DXHelper::ThrowIfFailed(m_commandList->Close());

	// Create Fence for synchronization
	DXHelper::ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	// @TODO Why do m_fenceValues[m_frameIndex]++?
	m_fenceValues[m_frameIndex]++;
	DXHelper::ThrowIfFailed(m_fence->SetName(L"Main Fence"));

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		Engine::GetLogger().LogError(LogCategory::D3D12, "Failed to create fence event.");
	}

	m_imguiManager = std::make_unique<DXImGuiManager>(
		Engine::GetWindow().GetWindow(), m_device, frameCount
	);

	directionalLightUniformBuffer = new DXConstantBuffer<DirectionalLightBatch>(m_device, frameCount);
	pointLightUniformBuffer = new DXConstantBuffer<PointLightBatch>(m_device, frameCount);

	// Initialize for work graphs
	if (m_workGraphsEnabled)
	{
		m_workGraphsContext->InitializeWorkGraphs();
	}

	// Initialize for compute shader
	//m_computeBuffer = std::make_unique<DXComputeBuffer>();
	//m_computeBuffer->InitComputeBuffer(m_device, "../Engine/shaders/hlsl/Compute.compute.hlsl", 1280, 720, m_srvHeap, m_renderTarget);

	WaitForGPU();

	SDL_ShowWindow(window);
}

void DXRenderManager::SetResize()
{
	m_isResize = true;
}

void DXRenderManager::OnResize()
{
	//OutputDebugStringA("OnResize: Entered.\n");

	SDL_GetWindowSizeInPixels(Engine::GetWindow().GetWindow(), &m_width, &m_height);

	if (m_width == 0 || m_height == 0)
	{
		//OutputDebugStringA("OnResize: Skipped due to 0 size.\n");
		return;
	}

	//OutputDebugStringA("OnResize: Waiting for GPU...\n");
	WaitForGPU();
	//OutputDebugStringA("OnResize: GPU wait finished.\n");

	for (auto& renderTarget : m_renderTargets)
	{
		renderTarget.Reset();
	}
	//OutputDebugStringA("OnResize: Old resources released.\n");

	DXHelper::ThrowIfFailed(m_swapChain->ResizeBuffers(
		frameCount,
		static_cast<UINT>(m_width),
		static_cast<UINT>(m_height),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0
	));
	//OutputDebugStringA("OnResize: ResizeBuffers successful.\n");

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < frameCount; ++i)
	{
		DXHelper::ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
		DXHelper::ThrowIfFailed(m_renderTargets[i]->SetName((L"Recreate Render Target " + std::to_wstring(i)).c_str()));
	}

	// Recreate Compute Shader
	//m_computeBuffer->OnResize(m_device, width, height, m_srvHeap, m_renderTarget);

	// Recreate FidelityFX
	m_postProcessContext->OnResize();

	// unique_ptr's release() == give up ownership, does not deallocate memory
	// unique_ptr's reset() == deallocate the memory
	m_renderTarget.reset();
	m_renderTarget = std::make_unique<DXRenderTarget>(
		m_device, Engine::GetWindow().GetWindow(),
		m_postProcessContext->GetFidelityFX()->GetRenderWidth(),
		m_postProcessContext->GetFidelityFX()->GetRenderHeight(),
		m_deferredRenderingEnabled
	);
	// Allocate SRV handle for tone mapping
	// @TODO This should be inside DXRenderTarget
	m_renderTarget->CreateSRV(m_hdrSrvHandle.first);

	m_skyboxRenderContext->OnResize();

	if (m_deferredRenderingEnabled)
	{
		m_gBufferContext->OnResize();
		m_globalLightingContext->OnResize();
		m_localLightingContext->OnResize();
		m_naiveLightingContext->OnResize();
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	//OutputDebugStringA("OnResize: Finished successfully.\n");

	UINT64 completedValue = m_fence->GetCompletedValue();
	for (auto& fenceValue : m_fenceValues)
	{
		fenceValue = completedValue;
	}
}

bool DXRenderManager::BeginRender(glm::vec3 bgColor)
{
	if (m_isResize)
	{
		OnResize();
		m_isResize = false;
		return false;
	}

	DXHelper::ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

	// Set the viewport and scissor rect
	// @TODO This is weird but FidelityFX class takes care of viewport size (display size, render size)
	uint32_t renderWidth = m_postProcessContext->GetFidelityFX()->GetRenderWidth();
	uint32_t renderHeight = m_postProcessContext->GetFidelityFX()->GetRenderHeight();
	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(renderWidth), static_cast<FLOAT>(renderHeight), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(renderWidth), static_cast<LONG>(renderHeight) };

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	float clearColor[4] = { bgColor.r, bgColor.g, bgColor.b, 1.f };
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_renderTarget->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	DXCommandListWrapper wrapper(m_commandList.Get());
	switch (rMode)
	{
	case RenderType::TwoDimension:
	{
		// @TODO Should ClearRenderTargetView be inside render context?
		auto* backBuffer = m_renderTargets[m_frameIndex].Get();
		// Main Render Target: PRESENT -> RENDER_TARGET
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier(1, &barrier);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_frameIndex), m_rtvDescriptorSize);
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		m_2dRenderContext->Execute(&wrapper);
	}
	break;
	case RenderType::ThreeDimension:
	{
		// Forward Rendering
		if (!m_deferredRenderingEnabled)
		{
			// @TODO Should ClearRenderTargetView be inside render context?
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderTarget->GetMSAARtvHeap()->GetCPUDescriptorHandleForHeapStart();
			// MSAA Target: RESOLVE_SOURCE -> RENDER_TARGET
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_commandList->ResourceBarrier(1, &barrier);
			m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			if (m_workGraphsEnabled && m_meshNodesEnabled) m_workGraphsContext->ExecuteWorkGraphs();
			else m_forwardRenderContext->Execute(&wrapper);
		}
		// Deferred Rendering
		else
		{
			// @TODO Should ClearRenderTargetView be inside render context?
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderTarget->GetHDRRtvHeap()->GetCPUDescriptorHandleForHeapStart();
			// HDR Render Target: COMMON -> RENDER_TARGET
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget->GetHDRRenderTarget().Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_commandList->ResourceBarrier(1, &barrier);
			clearColor[3] = 0.f; // Set alpha to 0 for discarding in lighting pass shader
			m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			m_gBufferContext->Execute(&wrapper);
			//m_naiveLightingContext->Execute(&wrapper);
			m_globalLightingContext->Execute(&wrapper);
			if (!m_meshletVisualization) m_localLightingContext->Execute(&wrapper);
		}
		if (m_skyboxEnabled) m_skyboxRenderContext->Execute(&wrapper);
		m_postProcessContext->Execute(&wrapper);
	}
	break;
	}

	m_imguiManager->Begin();

	return true;
}

void DXRenderManager::EndRender()
{
	//OutputDebugStringA("EndRender: Entered.\n");

	// Work Graphs Execution
	//if (m_workGraphsEnabled)
	//{
	//	m_workGraphsContext->ExecuteWorkGraphs();
	//	m_workGraphsContext->PrintWorkGraphsResults();
	//}

	// ImGui Render
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_frameIndex), m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	m_imguiManager->End(m_commandList);

	auto finalBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &finalBarrier);

	DXHelper::ThrowIfFailed(m_commandList->Close());
	//OutputDebugStringA("EndRender: Command list closed.\n");

	// Execute the command list
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	//OutputDebugStringA("EndRender: Command list executed.\n");

	//OutputDebugStringA("EndRender: Calling Present...\n");
	// Present(1, 0) = VSync On, Present(0, 0) = VSync Off
	DXHelper::ThrowIfFailed(m_swapChain->Present(1, 0));
	//OutputDebugStringA("EndRender: Present successful.\n");

	// Compute Shader Render
	//auto preResolveBarriers = {
	//	CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
	//};
	//m_commandList->ResourceBarrier(static_cast<UINT>(preResolveBarriers.size()), preResolveBarriers.begin());

	//// Process compute shader
	//m_computeBuffer->PostProcess(m_commandList, m_srvHeap, m_renderTarget, m_renderTargets[m_frameIndex]);

	//// ImGui Render
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_frameIndex), m_rtvDescriptorSize);
	//m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	//m_imguiManager->End(m_commandList);

	//auto finalBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//m_commandList->ResourceBarrier(1, &finalBarrier);

	//DXHelper::ThrowIfFailed(m_commandList->Close());
	////OutputDebugStringA("EndRender: Command list closed.\n");

	//// Execute the command list
	//ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	//m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	////OutputDebugStringA("EndRender: Command list executed.\n");

	////OutputDebugStringA("EndRender: Calling Present...\n");
	//DXHelper::ThrowIfFailed(m_swapChain->Present(1, 0));
	////OutputDebugStringA("EndRender: Present successful.\n");

	MoveToNextFrame();

	//OutputDebugStringA("EndRender: Finished successfully.\n");
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

void DXRenderManager::CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters) const
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_STATIC_SAMPLER_DESC samplers[2];
	// @TODO Take texture samplers as parameters to be customizable
	// Texture Sampler
	samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[0].MipLODBias = 0.0f;
	samplers[0].MaxAnisotropy = 16;
	samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplers[0].MinLOD = 0.0f;
	samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplers[0].ShaderRegister = 0;
	samplers[0].RegisterSpace = 1;
	samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// IBL Texture Sampler
	samplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[1].MipLODBias = 0.0f;
	samplers[1].MaxAnisotropy = 0;
	samplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplers[1].MinLOD = 0.0f;
	samplers[1].MaxLOD = D3D12_FLOAT32_MAX;
	samplers[1].ShaderRegister = 1;
	samplers[1].RegisterSpace = 2;
	samplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// @TODO What is root signature?
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(
		static_cast<UINT>(rootParameters.size()), rootParameters.data(),
		2, samplers,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signature, error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
	if (FAILED(hr))
	{
		if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
		Engine::GetLogger().LogError(LogCategory::D3D12, "Failed to serialize root signature.");
	}

	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
	DXHelper::ThrowIfFailed(rootSignature->SetName(L"Root Signature"));
}

// @TODO Should understand how this synchronization process works!
void DXRenderManager::WaitForGPU()
{
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));
	DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));

	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValues[m_frameIndex]++;
}

// @TODO Should understand how this synchronization process works!
void DXRenderManager::MoveToNextFrame()
{
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

std::string DXRenderManager::LogSrvBlocks()
{
	std::stringstream ss;
	ss << "m_availableSrvBlocks state: { ";
	for (const auto& pair : m_availableSrvBlocks)
	{
		ss << "[" << pair.first << ", size=" << pair.second << "] ";
	}
	ss << "}\n";
	return ss.str();
}

std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT> DXRenderManager::AllocateSrvHandles(const UINT& count)
{
	std::stringstream ss;
	ss << "====== SRV ALLOCATE Request: " << count << " descriptors ======\n";
	OutputDebugStringA(ss.str().c_str());
	ss.str("");

	UINT startIndex{ 0 };
	bool foundBlock{ false };

	for (auto it = m_availableSrvBlocks.begin(); it != m_availableSrvBlocks.end(); /*++it*/)
	{
		if (it->first < MAX_OBJECT_SIZE)
		{
			++it;
			continue;
		}

		if (it->second >= count)
		{
			startIndex = it->first;
			foundBlock = true;

			ss << "  [LOG] Found available block at index " << startIndex << " (size " << it->second << ").\n";

			if (it->second > count)
			{
				UINT remainingSize = it->second - count;
				UINT newStartIndex = startIndex + count;
				m_availableSrvBlocks.erase(it);
				m_availableSrvBlocks[newStartIndex] = remainingSize;
				ss << "  [LOG] Block split: Using " << count << ", remaining block at index " << newStartIndex << " (size " << remainingSize << ").\n";
			}
			else
			{
				m_availableSrvBlocks.erase(it);
				ss << "  [LOG] Block exact match: Using entire block.\n";
			}
			break;
		}
		else it++;
	}

	if (!foundBlock)
	{
		startIndex = m_srvDescriptorOffset;
		m_srvDescriptorOffset += count;

		ss << "  [LOG] No available block found. Allocating new block at index " << startIndex << ".\n";
		ss << "  [LOG] New m_srvDescriptorOffset: " << m_srvDescriptorOffset << "\n";

		if (m_srvDescriptorOffset >= m_srvHeap->GetDesc().NumDescriptors)
		{
			OutputDebugStringA("  [FATAL ERROR] SRV Descriptor Heap is full!\n");
			Engine::GetLogger().LogError(LogCategory::D3D12, "SRV Descriptor Heap is full.");
		}
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(
		m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
		static_cast<INT>(startIndex),
		m_srvDescriptorSize
	);

	ss << "  [LOG] Allocation successful. Returning startIndex: " << startIndex << "\n";
	ss << "  [LOG] " << LogSrvBlocks();
	ss << "====== SRV ALLOCATE End ===================================\n";
	OutputDebugStringA(ss.str().c_str());

	return { srvHandle, startIndex };
}

void DXRenderManager::DeallocateSrvBlock(UINT startIndex, UINT count)
{
	std::stringstream ss;
	ss << "====== SRV DEALLOCATE Request: index " << startIndex << ", count " << count << " ======\n";

	auto itAfter = m_availableSrvBlocks.upper_bound(startIndex);
	auto itBefore = m_availableSrvBlocks.end();

	// Check merge with previous block
	if (itAfter != m_availableSrvBlocks.begin())
	{
		itBefore = std::prev(itAfter);
		if (itBefore->first + itBefore->second == startIndex)
		{
			ss << "  [LOG] Merging with PREVIOUS block (index " << itBefore->first << ", size " << itBefore->second << ").\n";
			itBefore->second += count;
			startIndex = itBefore->first;
			count = itBefore->second;
		}
		else
		{
			itBefore = m_availableSrvBlocks.end();
		}
	}

	// Check merge wirth next block
	if (itAfter != m_availableSrvBlocks.end())
	{
		if (startIndex + count == itAfter->first)
		{
			ss << "  [LOG] Merging with NEXT block (index " << itAfter->first << ", size " << itAfter->second << ").\n";
			count += itAfter->second;
			m_availableSrvBlocks.erase(itAfter);
		}
	}

	ss << "  [LOG] Adding/Updating available block: index " << startIndex << ", count " << count << ".\n";
	m_availableSrvBlocks[startIndex] = count;

	ss << "  [LOG] " << LogSrvBlocks();
	ss << "====== SRV DEALLOCATE End =================================\n";
	OutputDebugStringA(ss.str().c_str());
}

void DXRenderManager::UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset)
{
	m_postProcessContext->UpdateScalePreset(effect, mode, preset);
}

void DXRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
	// Create Command Allocator
	ComPtr<ID3D12CommandAllocator> tempCommandAllocator;
	DXHelper::ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tempCommandAllocator)));
	DXHelper::ThrowIfFailed(tempCommandAllocator->SetName(L"Texture Command Allocator"));

	// Create Command List
	ComPtr<ID3D12GraphicsCommandList> tempCommandList;
	DXHelper::ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, tempCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&tempCommandList)));
	DXHelper::ThrowIfFailed(tempCommandList->SetName(L"Texture Command List"));
	DXHelper::ThrowIfFailed(tempCommandList->Close());

	// Create Fence
	ComPtr<ID3D12Fence> tempFence;
	DXHelper::ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tempFence)));
	DXHelper::ThrowIfFailed(tempFence->SetName(L"Texture Fence"));

	// Create Fence Event
	HANDLE tempFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (tempFenceEvent == nullptr)
	{
		Engine::GetLogger().LogError(LogCategory::D3D12, "Failed to create texture fence event.");
	}

	const auto& texture = textures.emplace_back(std::make_unique<DXTexture>());

	if (m_textureDescriptorOffset >= MAX_OBJECT_SIZE)
	{
		Engine::GetLogger().LogError(LogCategory::D3D12, "Texture Heap Full (Max 500)");
	}

	UINT srvIndex = m_textureDescriptorOffset++;
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(
		m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
		static_cast<INT>(srvIndex),
		m_srvDescriptorSize
	);
	auto deallocator = [](UINT index) {};

	DXHelper::ThrowIfFailed(tempCommandList->Reset(tempCommandAllocator.Get(), nullptr));
	texture->LoadTexture(
		m_device,
		tempCommandList,
		m_commandQueue,
		{ srvHandle, srvIndex },
		deallocator,
		tempFence,
		tempFenceEvent,
		false,
		path_,
		name_,
		flip
	);

	const int texId = static_cast<int>(srvIndex);
	texture->SetTextureID(texId);

	//tempCommandList->Close();
	//ID3D12CommandList* ppCommandLists[] = { tempCommandList.Get() };
	//m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//m_commandQueue->Signal(tempFence.Get(), 1);
	//if (tempFence->GetCompletedValue() < 1)
	//{
	//	tempFence->SetEventOnCompletion(1, tempFenceEvent);
	//	WaitForSingleObject(tempFenceEvent, INFINITE);
	//}
	CloseHandle(tempFenceEvent);
}

void DXRenderManager::ClearTextures()
{
	// Wait until GPU is synchronized before changing state
	/* @TODO This WaitForGPU() should be called before levelList.at(static_cast<int>(currentLevel))->End() is called
	 * Take a look at GameStateManager.cpp
	*/
	WaitForGPU();
	textures.clear();
	m_textureDescriptorOffset = 0;
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
	if (m_skyboxRenderContext) m_skyboxRenderContext->LoadSkybox(path);
}

void DXRenderManager::DeleteSkybox()
{
	if (m_skyboxRenderContext) m_skyboxRenderContext->DeleteSkybox();
}
