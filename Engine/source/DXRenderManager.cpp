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
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
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

	// Create Render Target Views (RTVs)
	UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < 2; ++i)
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
	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create command allocator.");
	}

	// Create Command List
	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create command list.");
	}

	m_commandList->Close();

	// Create Fence for synchronization
	hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create fence.");
	}

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		throw std::runtime_error("Failed to create fence event.");
	}

	SDL_ShowWindow(window_);
}

bool DXRenderManager::BeginRender(glm::vec3 bgColor)
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

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

	std::vector<Sprite*> sprites = Engine::Instance().GetSpriteManager().GetSprites();
	for (const auto& sprite : sprites)
	{
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &sprite->GetBufferWrapper()->GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer->GetView());
		m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
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

void DXRenderManager::CreateRootSignature()
{
	// @TODO What is root signature?
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
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
	if (SUCCEEDED(m_commandQueue->Signal(m_fence.Get(), m_fenceValue)))
	{
		if (m_fence->GetCompletedValue() < m_fenceValue)
		{
			if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
			{
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
		}
		m_fenceValue++;
	}
}

void DXRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
}

void DXRenderManager::DeleteWithIndex(int /*id*/)
{
	//Destroy Texture
	//for (auto t : textures)
	//	delete t;
	//textures.erase(textures.begin(), textures.end());
	//samplers.erase(samplers.begin(), samplers.end());
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
