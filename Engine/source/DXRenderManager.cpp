//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.cpp
#include "DXRenderManager.hpp"

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

	// Create Device
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter);
	DXHelper::ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
	DXHelper::ThrowIfFailed(m_device->SetName(L"Main Device"));

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

	//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	//dsvHeapDesc.NumDescriptors = 1;
	//dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	//dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	//DXHelper::ThrowIfFailed(m_dsvHeap->SetName(L"Depth/Stencil View Heap"));

	m_renderTarget = std::make_unique<DXRenderTarget>(m_device, window);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 505;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DXHelper::ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	DXHelper::ThrowIfFailed(m_srvHeap->SetName(L"Shader Resource View Heap"));

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

	// Create root signature and pipeline for 2D
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(3, {});
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 texSrvRange;
	texSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_OBJECT_SIZE, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[2].InitAsDescriptorTable(1, &texSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CreateRootSignature(m_rootSignature2D, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature2D->SetName(L"2D Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(TwoDimension::Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = m_renderTarget->GetMSAASampleCount();
	sampleDesc.Quality = m_renderTarget->GetMSAAQualityLevel();

	m_pipeline2D = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignature2D,
		std::filesystem::path("../Engine/shaders/hlsl/2D.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/2D.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{ positionLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		true,
		true,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	// Create root signature and pipeline for 3D
	rootParameters.clear();
	rootParameters.resize(8, {});
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[3].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[4].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[5].InitAsConstants(2, 5, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	rootParameters[6].InitAsDescriptorTable(1, &texSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_DESCRIPTOR_RANGE1 iblSrvRange;
	iblSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[7].InitAsDescriptorTable(1, &iblSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CreateRootSignature(m_rootSignature3D, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature3D->SetName(L"3D Root Signature"));

	positionLayout.offset = offsetof(ThreeDimension::QuantizedVertex, position);
	DXAttributeLayout normalLayout{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout uvLayout{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ThreeDimension::QuantizedVertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };
	DXAttributeLayout texSubIndexLayout{ "TEXCOORD", 1, DXGI_FORMAT_R32_SINT, 0, offsetof(ThreeDimension::QuantizedVertex, texSubIndex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	m_pipeline3D = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignature3D,
		std::filesystem::path("../Engine/shaders/hlsl/3D.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/3D.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{ positionLayout, normalLayout, uvLayout, texSubIndexLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		sampleDesc,
		true,
		true,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	m_pipeline3DLine = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignature3D,
		std::filesystem::path("../Engine/shaders/hlsl/3D.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/3D.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{ positionLayout, normalLayout, uvLayout, texSubIndexLayout },
		D3D12_FILL_MODE_WIREFRAME,
		D3D12_CULL_MODE_BACK,
		sampleDesc,
		true,
		true,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

#ifdef _DEBUG
	// Create root signature and pipeline for Normal 3D
	rootParameters.clear();
	rootParameters.resize(1, {});
	rootParameters[0].InitAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CreateRootSignature(m_rootSignature3DNormal, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature3DNormal->SetName(L"Normal 3D Root Signature"));

	positionLayout.format = DXGI_FORMAT_R32G32B32_FLOAT;
	positionLayout.offset = offsetof(ThreeDimension::NormalVertex, position);

	m_pipeline3DNormal = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignature3DNormal,
		std::filesystem::path("../Engine/shaders/hlsl/Normal3D.vert.hlsl"),
		std::filesystem::path("../Engine/shaders/hlsl/Normal3D.frag.hlsl"),
		std::initializer_list<DXAttributeLayout>{ positionLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		sampleDesc,
		true,
		true,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);
#endif
	
	// Create Command List
	DXHelper::ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipeline2D->GetPipelineState().Get(), IID_PPV_ARGS(&m_commandList)));
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
		throw std::runtime_error("Failed to create fence event.");
	}

	m_imguiManager = std::make_unique<DXImGuiManager>(
		Engine::GetWindow().GetWindow(), m_device, frameCount
	);

	directionalLightUniformBuffer = new DXConstantBuffer<DirectionalLightBatch>(m_device, frameCount);
	pointLightUniformBuffer = new DXConstantBuffer<PointLightBatch>(m_device, frameCount);

	WaitForGPU();

	SDL_ShowWindow(window);
}

void DXRenderManager::SetResize(const int width, const int height)
{
	m_width = width;
	m_height = height;
	m_isResize = true;
}

void DXRenderManager::OnResize(int width, int height)
{
	//OutputDebugStringA("OnResize: Entered.\n");

	if (width == 0 || height == 0)
	{
		//OutputDebugStringA("OnResize: Skipped due to 0 size.\n");
		return;
	}

	//OutputDebugStringA("OnResize: Waiting for GPU...\n");
	WaitForGPU();
	//OutputDebugStringA("OnResize: GPU wait finished.\n");

	for (UINT i = 0; i < frameCount; ++i)
	{
		m_renderTargets[i].Reset();
	}
	//OutputDebugStringA("OnResize: Old resources released.\n");

	DXHelper::ThrowIfFailed(m_swapChain->ResizeBuffers(
		frameCount,
		static_cast<UINT>(width),
		static_cast<UINT>(height),
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

	m_renderTarget.reset();
	m_renderTarget = std::make_unique<DXRenderTarget>(m_device, Engine::GetWindow().GetWindow());

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	//OutputDebugStringA("OnResize: Finished successfully.\n");

	UINT64 completedValue = m_fence->GetCompletedValue();
	for (UINT i = 0; i < frameCount; ++i)
	{
		m_fenceValues[i] = completedValue;
	}
}

bool DXRenderManager::BeginRender(glm::vec3 bgColor)
{
	if (m_isResize)
	{
		OnResize(m_width, m_height);
		m_isResize = false;
		return false;
	}

	DXHelper::ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

	ID3D12PipelineState* initialState = rMode == RenderType::TwoDimension ? m_pipeline2D->GetPipelineState().Get() :
		pMode == PolygonType::FILL ? m_pipeline3D->GetPipelineState().Get() : m_pipeline3DLine->GetPipelineState().Get();
	DXHelper::ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), initialState));

	// Set the viewport and scissor rect
	int width, height;
	SDL_GetWindowSizeInPixels(Engine::GetWindow().GetWindow(), &width, &height);
	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, width, height };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderTarget->GetMSAARtvHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_renderTarget->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	// @TODO What does OMSetRenderTargets do?
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	const float clearColor[] = { bgColor.r, bgColor.g, bgColor.b, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	std::vector<Sprite*> sprites = Engine::Instance().GetSpriteManager().GetSprites();
	switch (rMode)
	{
	case RenderType::TwoDimension:
	{
		m_commandList->SetGraphicsRootSignature(m_rootSignature2D.Get());
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D12DescriptorHeap* ppHeaps2D[] = { m_srvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps2D), ppHeaps2D);
		m_commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

		for (const auto& sprite : sprites)
		{
			auto& buffer = sprite->GetBufferWrapper()->GetBuffer<BufferWrapper::DXBuffer>();
			auto& constantBuffer = sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>();

			// Update Constant Buffer
			constantBuffer.vertexUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform, sizeof(TwoDimension::VertexUniform), m_frameIndex);
			constantBuffer.fragmentUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform, sizeof(TwoDimension::FragmentUniform), m_frameIndex);

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
	}
	break;
	case RenderType::ThreeDimension:
	{
		m_commandList->SetGraphicsRootSignature(m_rootSignature3D.Get());
		ID3D12DescriptorHeap* ppHeaps3D[] = { m_srvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps3D), ppHeaps3D);

		for (const auto& sprite : sprites)
		{
			m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			auto& buffer = sprite->GetBufferWrapper()->GetBuffer<BufferWrapper::DXBuffer>();
			auto& constantBuffer = sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>();

			// Update Constant Buffer
			constantBuffer.vertexUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform, sizeof(ThreeDimension::VertexUniform), m_frameIndex);
			constantBuffer.fragmentUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform, sizeof(ThreeDimension::FragmentUniform), m_frameIndex);
			constantBuffer.materialUniformBuffer->UpdateConstant(&sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().material, sizeof(ThreeDimension::Material), m_frameIndex);

			if (directionalLightUniformBuffer && !directionalLightUniforms.empty())
			{
				directionalLightUniformBuffer->UpdateConstant(directionalLightUniforms.data(), sizeof(ThreeDimension::DirectionalLightUniform) * directionalLightUniforms.size(), m_frameIndex);
				m_commandList->SetGraphicsRootConstantBufferView(3, directionalLightUniformBuffer->GetGPUVirtualAddress(m_frameIndex));
			}
			if (pointLightUniformBuffer && !pointLightUniforms.empty())
			{
				pointLightUniformBuffer->UpdateConstant(pointLightUniforms.data(), sizeof(ThreeDimension::PointLightUniform) * pointLightUniforms.size(), m_frameIndex);
				m_commandList->SetGraphicsRootConstantBufferView(4, pointLightUniformBuffer->GetGPUVirtualAddress(m_frameIndex));
			}

			pushConstants.activeDirectionalLight = static_cast<int>(directionalLightUniforms.size());
			pushConstants.activePointLight = static_cast<int>(pointLightUniforms.size());
			m_commandList->SetGraphicsRoot32BitConstants(5, 2, &pushConstants, 0);

			m_commandList->SetGraphicsRootDescriptorTable(6, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

			if (skyboxEnabled && m_skybox)
			{
				m_commandList->SetGraphicsRootDescriptorTable(7, m_skybox->GetIrradianceMapSrv());
			}

			// Bind Vertex Buffer & Index Buffer
			D3D12_VERTEX_BUFFER_VIEW vbv = buffer.vertexBuffer->GetView();
			D3D12_INDEX_BUFFER_VIEW ibv = buffer.indexBuffer->GetView();
			m_commandList->IASetVertexBuffers(0, 1, &vbv);
			m_commandList->IASetIndexBuffer(&ibv);
			// Bind constant buffers to root signature
			m_commandList->SetGraphicsRootConstantBufferView(0, constantBuffer.vertexUniformBuffer->GetGPUVirtualAddress(m_frameIndex));
			m_commandList->SetGraphicsRootConstantBufferView(1, constantBuffer.fragmentUniformBuffer->GetGPUVirtualAddress(m_frameIndex));
			m_commandList->SetGraphicsRootConstantBufferView(2, constantBuffer.materialUniformBuffer->GetGPUVirtualAddress(m_frameIndex));

			m_commandList->DrawIndexedInstanced(static_cast<UINT>(sprite->GetBufferWrapper()->GetIndices().size()), 1, 0, 0, 0);

#ifdef _DEBUG
			if (isDrawNormals)
			{
				m_commandList->SetPipelineState(m_pipeline3DNormal->GetPipelineState().Get());
				m_commandList->SetGraphicsRootSignature(m_rootSignature3DNormal.Get());
				m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

				auto& vertexUniform = sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
				glm::mat4 modelToNDC = vertexUniform.projection * vertexUniform.view * vertexUniform.model;
				m_commandList->SetGraphicsRoot32BitConstants(0, 16, &modelToNDC, 0);

				vbv = buffer.normalVertexBuffer->GetView();
				m_commandList->IASetVertexBuffers(0, 1, &vbv);

				m_commandList->DrawInstanced(static_cast<UINT>(sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices.size()), 1, 0, 0);

				switch (pMode)
				{
				case PolygonType::FILL:
					m_commandList->SetPipelineState(m_pipeline3D->GetPipelineState().Get());
					break;
				case PolygonType::LINE:
					m_commandList->SetPipelineState(m_pipeline3DLine->GetPipelineState().Get());
					break;
				}
				m_commandList->SetGraphicsRootSignature(m_rootSignature3D.Get());
			}
#endif
		}

		if (skyboxEnabled)
		{
			m_commandList->SetPipelineState(m_pipelineSkybox->GetPipelineState().Get());
			m_commandList->SetGraphicsRootSignature(m_rootSignatureSkybox.Get());
			m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ID3D12DescriptorHeap* ppHeapsSkybox[] = { m_srvHeap.Get() };
			m_commandList->SetDescriptorHeaps(_countof(ppHeapsSkybox), ppHeapsSkybox);

			glm::mat4 worldToNDC[2] = { Engine::GetCameraManager().GetViewMatrix(), Engine::GetCameraManager().GetProjectionMatrix() };
			m_commandList->SetGraphicsRoot32BitConstants(0, 32, &worldToNDC, 0);
			m_commandList->SetGraphicsRootDescriptorTable(1, m_skybox->GetCubemapSrv());

			D3D12_VERTEX_BUFFER_VIEW vbv = m_skyboxVertexBuffer->GetView();
			m_commandList->IASetVertexBuffers(0, 1, &vbv);

			m_commandList->DrawInstanced(36, 1, 0, 0);
		}
	}
	break;
	}

	m_imguiManager->Begin();

	return true;
}

void DXRenderManager::EndRender()
{
	//OutputDebugStringA("EndRender: Entered.\n");

	auto preResolveBarriers = {
	CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget->GetMSAARenderTarget().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
	CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST)
	};
	m_commandList->ResourceBarrier(static_cast<UINT>(preResolveBarriers.size()), preResolveBarriers.begin());

	m_commandList->ResolveSubresource(m_renderTargets[m_frameIndex].Get(), 0, m_renderTarget->GetMSAARenderTarget().Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	auto postResolveBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &postResolveBarrier);

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
	DXHelper::ThrowIfFailed(m_swapChain->Present(1, 0));
	//OutputDebugStringA("EndRender: Present successful.\n");

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

void DXRenderManager::CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_STATIC_SAMPLER_DESC samplers[2];

	// Normal Texture Sampler
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
		throw std::runtime_error("Failed to serialize root signature.");
	}

	DXHelper::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void DXRenderManager::WaitForGPU()
{
	DXHelper::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));
	DXHelper::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));

	WaitForSingleObject(m_fenceEvent, INFINITE);
	m_fenceValues[m_frameIndex]++;
}

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
		throw std::runtime_error("Failed to create texture fence event.");
	}

	const auto& texture = textures.emplace_back(std::make_unique<DXTexture>());

	const int texId = static_cast<int>(textures.size() - 1);
	texture->SetTextureID(texId);

	DXHelper::ThrowIfFailed(tempCommandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));
	texture->LoadTexture(
		m_device,
		tempCommandList,
		m_srvHeap,
		m_commandQueue,
		tempFence,
		tempFenceEvent,
		texture->GetTextrueId(),
		false,
		path_,
		name_,
		flip
	);

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
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	m_skyboxVertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(float)) * 3, static_cast<UINT>(sizeof(float)) * 108, skyboxVertices);

	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(2, {});
	rootParameters[0].InitAsConstants(32, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	CD3DX12_DESCRIPTOR_RANGE1 texSrvRange;
	texSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	rootParameters[1].InitAsDescriptorTable(1, &texSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CreateRootSignature(m_rootSignatureSkybox, rootParameters);

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = m_renderTarget->GetMSAASampleCount();
	sampleDesc.Quality = m_renderTarget->GetMSAAQualityLevel();

	m_pipelineSkybox = std::make_unique<DXPipeLine>(
		m_device,
		m_rootSignatureSkybox,
		"../Engine/shaders/hlsl/Skybox.vert.hlsl",
		"../Engine/shaders/hlsl/Skybox.frag.hlsl",
		std::initializer_list<DXAttributeLayout>{ positionLayout },
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		sampleDesc,
		true,
		true,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	WaitForGPU();
	m_skybox = std::make_unique<DXSkybox>(m_device, m_commandQueue, m_srvHeap, MAX_OBJECT_SIZE);
	m_skybox->Initialize(path);

	skyboxEnabled = true;
}

void DXRenderManager::DeleteSkybox()
{
	m_skyboxVertexBuffer.reset();

	m_rootSignatureSkybox.Reset();

	m_pipelineSkybox.reset();

	m_skybox.reset();

	skyboxEnabled = false;
}
