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

#include "BasicComponents/Sprite.hpp"

#define MAX_OBJECT_SIZE 500
#define MAX_LIGHT_SIZE 10

using Microsoft::WRL::ComPtr;

class DXRenderManager : public RenderManager
{
public:
	DXRenderManager() { gMode = GraphicsMode::DX; }
	~DXRenderManager() override;

	DXRenderManager(const DXRenderManager&) = delete;
	DXRenderManager& operator=(const DXRenderManager&) = delete;
	DXRenderManager(const DXRenderManager&&) = delete;
	DXRenderManager& operator=(const DXRenderManager&&) = delete;

	void Initialize(SDL_Window* window);
	void SetResize(const int width, const int height);
	int m_width, m_height;
	bool m_isResize{ false };
	void OnResize(const int width, const int height);
	void WaitForGPU();

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;
private:
	// Initialize DirectX 12 components
	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter);
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters);
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

	static constexpr UINT frameCount = 2;

	// m = member
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[frameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[frameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature2D;
	ComPtr<ID3D12RootSignature> m_rootSignature3D;
	// rtv = Render Target View
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// cbv/srv = Constant Buffer View / Shader Resource View
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	UINT m_rtvDescriptorSize{ 0 };
	//UINT m_srvDescriptorSize{ 0 };

	UINT m_frameIndex{ 0 };
	HANDLE m_fenceEvent{ nullptr };
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[frameCount]{};

	std::unique_ptr<DXPipeLine> m_pipeline2D;
	std::unique_ptr<DXPipeLine> m_pipeline3D;
	std::unique_ptr<DXPipeLine> m_pipeline3DLine;
#ifdef _DEBUG
	std::unique_ptr<DXPipeLine> m_pipeline3DNormal;
	ComPtr<ID3D12RootSignature> m_rootSignature3DNormal;
#endif

	// MSAA
	// Depth
	std::unique_ptr<DXRenderTarget> m_renderTarget;

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

	void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().indexBuffer = std::make_unique<DXIndexBuffer>(m_device, m_commandQueue, &indices);
		if (rMode == RenderType::TwoDimension)
		{
			auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(TwoDimension::Vertex)), static_cast<UINT>(sizeof(TwoDimension::Vertex) * vertices.size()), vertices.data());

			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>().vertexUniformBuffer = std::make_unique<DXConstantBuffer<TwoDimension::VertexUniform>>(m_device, frameCount);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>().fragmentUniformBuffer = std::make_unique<DXConstantBuffer<TwoDimension::FragmentUniform>>(m_device, frameCount);
		}
		else if (rMode == RenderType::ThreeDimension)
		{
			auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(ThreeDimension::QuantizedVertex)), static_cast<UINT>(sizeof(ThreeDimension::QuantizedVertex) * vertices.size()), vertices.data());
#ifdef _DEBUG
			auto& normalVertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().normalVertexBuffer = std::make_unique<DXVertexBuffer>(m_device, m_commandQueue, static_cast<UINT>(sizeof(ThreeDimension::NormalVertex)), static_cast<UINT>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
#endif

			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().vertexUniformBuffer = std::make_unique<DXConstantBuffer<ThreeDimension::VertexUniform>>(m_device, frameCount);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().fragmentUniformBuffer = std::make_unique<DXConstantBuffer<ThreeDimension::FragmentUniform>>(m_device, frameCount);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().materialUniformBuffer = std::make_unique<DXConstantBuffer<ThreeDimension::Material>>(m_device, frameCount);
		}
	}

	// Deferred Deletion
	std::vector<std::pair<Sprite*, UINT64>> m_deletionQueue;
	void SafeDelete(Sprite* sprite)
	{
		if (sprite) m_deletionQueue.emplace_back(std::pair<Sprite*, UINT64>{ sprite, m_frameIndex });
	}
	//void ProcessDeletionQueue()
	//{
	//	const UINT64 gpuCompletedFrame = m_fence->GetCompletedValue();

	//	m_deletionQueue.erase(
	//		std::remove_if(m_deletionQueue.begin(), m_deletionQueue.end(),
	//			[gpuCompletedFrame](const auto& item)
	//			{
	//				return item.second <= gpuCompletedFrame;
	//			}
	//		),
	//		m_deletionQueue.end()
	//	);
	//}
	void ProcessDeletionQueue();
	//{
	//	const UINT64 gpuCompletedFrame = m_fence->GetCompletedValue();

	//	auto it = m_deletionQueue.begin();
	//	while (it != m_deletionQueue.end())
	//	{
	//		if (it->second <= gpuCompletedFrame)
	//		{
	//			delete it->first;
	//			Engine::GetObjectManager().ProcessFunctionQueue();
	//			Engine::GetObjectManager().DeleteObjectsFromList();

	//			it = m_deletionQueue.erase(it);
	//		}
	//		else
	//		{
	//			++it;
	//		}
	//	}
	//}

	//--------------------2D Render--------------------//
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;

	DXTexture* GetTexture(const std::string& name) const;
	const std::vector<std::unique_ptr<DXTexture>>& GetTextures() { return textures; }

	//--------------------3D Render--------------------//

	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
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
	struct alignas(16) PushConstants
	{
		int activeDirectionalLight;
		int activePointLight;
	} pushConstants;

	//Skybox
	std::unique_ptr<DXVertexBuffer> m_skyboxVertexBuffer;
	ComPtr<ID3D12RootSignature> m_rootSignatureSkybox;
	std::unique_ptr<DXPipeLine> m_pipelineSkybox;
	std::unique_ptr<DXSkybox> m_skybox;
};
