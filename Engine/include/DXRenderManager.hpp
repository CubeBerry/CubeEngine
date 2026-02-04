//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.hpp
#pragma once
#include "RenderManager.hpp"

#include <dxgi1_6.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#include "DebugTools.hpp"
#endif

#include "DXPipeLine.hpp"
#include "DXTexture.hpp"
#include "DXImGuiManager.hpp"
#include "DXSkybox.hpp"
#include "DXRenderTarget.hpp"
#include "DXComputeBuffer.hpp"
#include "DXWorkGraphsContext.hpp"
#include "DXIndexBuffer.hpp"
#include "DXConstantBuffer.hpp"
#include "DX2DRenderContext.hpp"
#include "DXForwardRenderContext.hpp"
#include "DXNaiveLightingContext.hpp"
#include "DXGlobalLightingContext.hpp"
#include "DXLocalLightingContext.hpp"
#include "DXGBufferContext.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "DXPostProcessContext.hpp"

#define MAX_OBJECT_SIZE 500
#define MAX_LIGHT_SIZE 10

using Microsoft::WRL::ComPtr;

class DXRenderManager : public RenderManager
{
	friend class DX2DRenderContext;
	friend class DXForwardRenderContext;
	friend class DXGBufferContext;
	friend class DXNaiveLightingContext;
	friend class DXGlobalLightingContext;
	friend class DXLocalLightingContext;
	friend class DXSkyboxRenderContext;
	friend class DXPostProcessContext;
	// @TODO Maybe would need to remove friend class later and modify IWorkGraphsContext functions to use parameters
	friend class DXWorkGraphsContext;
public:
	DXRenderManager() { gMode = GraphicsMode::DX; }
	~DXRenderManager() override;

	DXRenderManager(const DXRenderManager&) = delete;
	DXRenderManager& operator=(const DXRenderManager&) = delete;
	DXRenderManager(const DXRenderManager&&) = delete;
	DXRenderManager& operator=(const DXRenderManager&&) = delete;

	void Initialize(SDL_Window* window);
	void SetResize();

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;

	DXPostProcessContext* GetPostProcessContext() const { return m_postProcessContext.get(); }
private:
	int m_width, m_height;
	bool m_isResize{ false };
	void OnResize();
	void WaitForGPU();

	// Initialize DirectX 12 components
	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter);
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters) const;
	void MoveToNextFrame();

	struct LiveObjectReporter
	{
		~LiveObjectReporter()
		{
#ifdef _DEBUG
			ComPtr<IDXGIDebug1> dxgiDebug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
			{
				dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
			}
#endif
		}
	} reporter;

	// Descriptor Start Index, Block Size
	std::map<UINT, UINT> m_availableSrvBlocks;
	// SRV Handle, Descriptor Index
	// @TODO Make struct that contains DeallocateSrvBlock inside destructor later
	std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT> AllocateSrvHandles(const UINT& count = 1);
	//void DeallocateSrvBlock(UINT startIndex, UINT count);
	std::string LogSrvBlocks();

	static constexpr UINT frameCount = 2;

	// m = member
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device14> m_device;
	// Main Render Target Views for Swap Chain
	ComPtr<ID3D12Resource> m_renderTargets[frameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[frameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature2D;
	// rtv = Render Target View
	// RTV Heap for Swap Chain
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// cbv/srv = Constant Buffer View / Shader Resource View
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12GraphicsCommandList10> m_commandList;

	UINT m_rtvDescriptorSize{ 0 };
	UINT m_srvDescriptorSize{ 0 };
	UINT m_textureDescriptorOffset{ 0 };
	UINT m_srvDescriptorOffset{ 0 };

	UINT m_frameIndex{ 0 };
	HANDLE m_fenceEvent{ nullptr };
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[frameCount]{};

	// 2D Render Context
	std::unique_ptr<DX2DRenderContext> m_2dRenderContext;
	// Forward Render Context
	std::unique_ptr<DXForwardRenderContext> m_forwardRenderContext;
	// G-Buffer Render Context
	std::unique_ptr<DXGBufferContext> m_gBufferContext;
	DXGBufferContext* GetGBufferContext() const { return m_gBufferContext.get(); }
	// Naive Lighting Context
	std::unique_ptr<DXNaiveLightingContext> m_naiveLightingContext;
	// Global (Directional) Lighting Context
	std::unique_ptr<DXGlobalLightingContext> m_globalLightingContext;
	// Local (Point) Lighting Context
	std::unique_ptr<DXLocalLightingContext> m_localLightingContext;
	// Skybox Render Context
	std::unique_ptr<DXSkyboxRenderContext> m_skyboxRenderContext;
	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
	// Post-Process Render Context
	std::unique_ptr<DXPostProcessContext> m_postProcessContext;
	// Work Graphs Context
	std::unique_ptr<DXWorkGraphsContext> m_workGraphsContext;

	// HDR, MSAA, Depth
	std::unique_ptr<DXRenderTarget> m_renderTarget;
	// @TODO This should be inside DXRenderTarget
	std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT> m_hdrSrvHandle;

	// Compute Shader
	// @TODO Make Compute Shader Context Later
	std::unique_ptr<DXComputeBuffer> m_computeBuffer;

#if USE_NSIGHT_AFTERMATH
	// App-managed marker functionality
	UINT64 m_frameCounter{ 0 };
	GpuCrashTracker::MarkerMap m_markerMap;

	// Nsight Aftermath instrumentation
	GFSDK_Aftermath_ContextHandle m_hAftermathCommandListContext{ nullptr };
	GpuCrashTracker m_gpuCrashTracker{ m_markerMap };
#endif

	std::unique_ptr<DXImGuiManager> m_imguiManager;
public:
	//--------------------Common--------------------//
	void ClearTextures() override;
	
	// @TODO Should this function in public due to deallocating srv block in BufferWrapper's destructor?
	void DeallocateSrvBlock(UINT startIndex, UINT count);

	void InitializeDynamicBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>()->indexBuffer = std::make_unique<DXIndexBuffer>(m_device, m_commandQueue, &indices);
		if (rMode == RenderType::TwoDimension)
		{
			auto* sprite = bufferWrapper.GetData<BufferWrapper::DynamicSprite2D>();

			auto& vertices = sprite->vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>()->vertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(TwoDimension::Vertex)), static_cast<UINT>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

			sprite->SetVertexUniformBuffer(std::make_unique<DXConstantBuffer<TwoDimension::VertexUniform>>(m_device, frameCount));
			sprite->SetFragmentUniformBuffer(std::make_unique<DXConstantBuffer<TwoDimension::FragmentUniform>>(m_device, frameCount));
		}
		else if (rMode == RenderType::ThreeDimension)
		{
			auto* sprite = bufferWrapper.GetData<BufferWrapper::DynamicSprite3DMesh>();
			auto* buffer = bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>();

			auto& vertices = sprite->vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>()->vertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(ThreeDimension::QuantizedVertex)), static_cast<UINT>(sizeof(ThreeDimension::QuantizedVertex) * vertices.size()), vertices.data());
#ifdef _DEBUG
			auto& normalVertices = sprite->normalVertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>()->normalVertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(ThreeDimension::NormalVertex)), static_cast<UINT>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
#endif

			sprite->SetVertexUniformBuffer(std::make_unique<DXConstantBuffer<ThreeDimension::VertexUniform>>(m_device, frameCount));
			sprite->SetFragmentUniformBuffer(std::make_unique<DXConstantBuffer<ThreeDimension::FragmentUniform>>(m_device, frameCount));
			sprite->SetMaterialUniformBuffer(std::make_unique<DXConstantBuffer<ThreeDimension::Material>>(m_device, frameCount));

			if (m_meshShaderEnabled)
			{
				buffer->srvHandle = AllocateSrvHandles(4);

				CD3DX12_CPU_DESCRIPTOR_HANDLE uniqueVertexSrvHandle(buffer->srvHandle.first, 0, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				buffer->uniqueVertexBuffer = std::make_unique<Meshlet::DynamicUniqueVertexBuffer>(m_device, m_commandQueue, vertices, uniqueVertexSrvHandle);

				CD3DX12_CPU_DESCRIPTOR_HANDLE meshletSrvHandle(buffer->srvHandle.first, 1, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				buffer->meshletBuffer = std::make_unique<Meshlet::MeshletBuffer>(m_device, m_commandQueue, sprite->meshlets, meshletSrvHandle);

				CD3DX12_CPU_DESCRIPTOR_HANDLE uniqueVertexIndexSrvHandle(buffer->srvHandle.first, 2, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				buffer->uniqueVertexIndexBuffer = std::make_unique<Meshlet::UniqueVertexIndexBuffer>(m_device, m_commandQueue, sprite->uniqueVertexIndices, uniqueVertexIndexSrvHandle);

				CD3DX12_CPU_DESCRIPTOR_HANDLE primitiveIndexSrvHandle(buffer->srvHandle.first, 3, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				buffer->primitiveIndexBuffer = std::make_unique<Meshlet::PrimitiveIndexBuffer>(m_device, m_commandQueue, sprite->primitiveIndices, primitiveIndexSrvHandle);
			}
		}
	}
	void InitializeGlobalUniformBuffers(BufferWrapper& bufferWrapper)
	{
		auto* sprite = bufferWrapper.GetData<BufferWrapper::StaticSprite3D>();
		auto* buffer = bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>();

		buffer->srvHandle = AllocateSrvHandles(3);
		CD3DX12_CPU_DESCRIPTOR_HANDLE vertexUniformSrvHandle(buffer->srvHandle.first, 0, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		sprite->SetVertexUniformBuffer(std::make_unique<DXStructuredBuffer<ThreeDimension::VertexUniform>>(m_device, m_commandQueue, sprite->vertexUniforms, vertexUniformSrvHandle));
		CD3DX12_CPU_DESCRIPTOR_HANDLE fragmentUniformSrvHandle(buffer->srvHandle.first, 1, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		sprite->SetFragmentUniformBuffer(std::make_unique<DXStructuredBuffer<ThreeDimension::FragmentUniform>>(m_device, m_commandQueue, sprite->fragmentUniforms, fragmentUniformSrvHandle));
		CD3DX12_CPU_DESCRIPTOR_HANDLE materialUniformIndexSrvHandle(buffer->srvHandle.first, 2, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		sprite->SetMaterialUniformBuffer(std::make_unique<DXStructuredBuffer<ThreeDimension::Material>>(m_device, m_commandQueue, sprite->materials, materialUniformIndexSrvHandle));
	}
	void InitializeStaticBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices)
	{
		// Initialize Buffers
		auto* sprite = bufferWrapper.GetData<BufferWrapper::StaticSprite3D>();
		auto* buffer = bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>();

		buffer->vertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(ThreeDimension::StaticQuantizedVertex)), static_cast<UINT>(sizeof(ThreeDimension::StaticQuantizedVertex) * sprite->vertices.size()), sprite->vertices.data());
		buffer->indexBuffer = std::make_unique<DXIndexBuffer>(m_device, m_commandQueue, &indices);

		if (m_meshShaderEnabled)
		{
			buffer->srvHandle = AllocateSrvHandles(4);

			CD3DX12_CPU_DESCRIPTOR_HANDLE uniqueVertexSrvHandle(buffer->srvHandle.first, 0, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			buffer->uniqueStaticVertexBuffer = std::make_unique<Meshlet::StaticUniqueVertexBuffer>(m_device, m_commandQueue, sprite->vertices, uniqueVertexSrvHandle);

			CD3DX12_CPU_DESCRIPTOR_HANDLE meshletSrvHandle(buffer->srvHandle.first, 1, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			buffer->meshletBuffer = std::make_unique<Meshlet::MeshletBuffer>(m_device, m_commandQueue, sprite->meshlets, meshletSrvHandle);

			CD3DX12_CPU_DESCRIPTOR_HANDLE uniqueVertexIndexSrvHandle(buffer->srvHandle.first, 2, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			buffer->uniqueVertexIndexBuffer = std::make_unique<Meshlet::UniqueVertexIndexBuffer>(m_device, m_commandQueue, sprite->uniqueVertexIndices, uniqueVertexIndexSrvHandle);

			CD3DX12_CPU_DESCRIPTOR_HANDLE primitiveIndexSrvHandle(buffer->srvHandle.first, 3, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			buffer->primitiveIndexBuffer = std::make_unique<Meshlet::PrimitiveIndexBuffer>(m_device, m_commandQueue, sprite->primitiveIndices, primitiveIndexSrvHandle);
		}
	}

	// Deferred Deletion
	// @TODO Make OpenGL, Vulkan version of SafeDelete function and remove ProcessFunctionQueue(), DeleteObjectsFromList()
	void SafeDelete(std::unique_ptr<BufferWrapper> bufferWrapper)
	{
		if (!bufferWrapper) return;

		auto srvHandle = bufferWrapper->GetBuffer<BufferWrapper::DXBuffer>()->srvHandle;
		const UINT srvCount = 4;

		const UINT64 queuedFrame = m_fenceValues[m_frameIndex];
		// shared count == 1
		auto bufferHolder = std::make_shared<std::unique_ptr<BufferWrapper>>(std::move(bufferWrapper));
		// shared count == 2
		QueueDeferredFunction([this, bufferHolder, queuedFrame, srvHandle, srvCount]() -> bool
			{
				if (queuedFrame <= m_fence->GetCompletedValue())
				{
					DeallocateSrvBlock(srvHandle.second, srvCount);
					return true;
				}
				return false;
				// shared count--
			}
		);
		// shared count--
	}

	// FidelityFX CAS
	void UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset) override;

	//--------------------2D Render--------------------//
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;

	DXTexture* GetTexture(const std::string& name) const;
	const std::vector<std::unique_ptr<DXTexture>>& GetTextures() { return textures; }

	//--------------------3D Render--------------------//
private:
	//--------------------Common--------------------//
	std::vector<std::unique_ptr<DXTexture>> textures;

	//Lighting
	struct DirectionalLightBatch
	{
		ThreeDimension::DirectionalLightUniform lights[MAX_LIGHT_SIZE];
	};
	struct PointLightBatch
	{
		ThreeDimension::PointLightUniform lights[MAX_LIGHT_SIZE];
	};
	DXConstantBuffer<DirectionalLightBatch>* directionalLightUniformBuffer{ nullptr };
	DXConstantBuffer<PointLightBatch>* pointLightUniformBuffer{ nullptr };
	// Push Constants for forward rendering
	struct alignas(16) PushConstants
	{
		int activeDirectionalLight;
		int activePointLight;
	} pushConstants;
};
