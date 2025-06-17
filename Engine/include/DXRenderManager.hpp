//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.hpp
#pragma once
#include "RenderManager.hpp"

#include <dxgi1_6.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "DXPipeLine.hpp"
#include "DXTexture.hpp"
#include "DXImGuiManager.hpp"

// @TODO temporal forward declaration classes for DirectX
class DXSkybox;

using Microsoft::WRL::ComPtr;

class DXRenderManager : public RenderManager
{
public:
	DXRenderManager() { gMode = GraphicsMode::DX; }
	~DXRenderManager() override;
	void Initialize(SDL_Window* window);
	void OnResize(int width, int height);

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;
private:
	// Initialize DirectX 12 components
	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter);
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters);
	void WaitForGPU();
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
#ifdef _DEBUG
	ComPtr<ID3D12RootSignature> m_rootSignature3DNormal;
#endif
	// rtv = Render Target View
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// dsv = Depth Stencil View
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	// cbv/srv = Constant Buffer View / Shader Resource View
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_depthStencil;

	UINT m_rtvDescriptorSize{ 0 };
	UINT m_cbvSrvDescriptorSize{ 0 };

	UINT m_frameIndex{ 0 };
	HANDLE m_fenceEvent{ nullptr };
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[frameCount]{};

	std::unique_ptr<DXPipeLine> m_pipeline2D;
	std::unique_ptr<DXPipeLine> m_pipeline3D;
#ifdef _DEBUG
	std::unique_ptr<DXPipeLine> m_pipeline3DNormal;
#endif

	std::unique_ptr<DXImGuiManager> m_imguiManager;
public:
	//--------------------Common--------------------//
	void ClearTextures() override;

	void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().indexBuffer = new DXIndexBuffer(m_device, &indices);
		if (rMode == RenderType::TwoDimension)
		{
			auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = new DXVertexBuffer(m_device, sizeof(TwoDimension::Vertex), sizeof(TwoDimension::Vertex) * static_cast<UINT>(vertices.size()), vertices.data());

			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>().vertexUniformBuffer = new DXConstantBuffer<TwoDimension::VertexUniform>(m_device, frameCount);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>().fragmentUniformBuffer = new DXConstantBuffer<TwoDimension::FragmentUniform>(m_device, frameCount);
		}
		else if (rMode == RenderType::ThreeDimension)
		{
			auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertices;
			auto& normalVertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = new DXVertexBuffer(m_device, sizeof(ThreeDimension::Vertex), sizeof(ThreeDimension::Vertex) * static_cast<UINT>(vertices.size()), vertices.data());
#ifdef _DEBUG
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().normalVertexBuffer = new DXVertexBuffer(m_device, sizeof(ThreeDimension::NormalVertex), sizeof(ThreeDimension::Vertex) * static_cast<UINT>(normalVertices.size()), normalVertices.data());
#endif

			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().vertexUniformBuffer = new DXConstantBuffer<ThreeDimension::VertexUniform>(m_device, frameCount);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().fragmentUniformBuffer = new DXConstantBuffer<ThreeDimension::FragmentUniform>(m_device, frameCount);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().materialUniformBuffer = new DXConstantBuffer<ThreeDimension::Material>(m_device, frameCount);
		}
	}

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

#ifdef _DEBUG
	//DXShader glNormal3DShader;
#endif

	//Lighting
	DXConstantBuffer<ThreeDimension::DirectionalLightUniform>* directionalLightUniformBuffer{ nullptr };
	DXConstantBuffer<ThreeDimension::PointLightUniform>* pointLightUniformBuffer{ nullptr };
	struct alignas(16) PushConstants
	{
		int activeDirectionalLight;
		int activePointLight;
	} pushConstants;

	//Skybox
	//DXVertexBuffer* skyboxVertexBuffer{ nullptr };
	//DXShader skyboxShader;
	//DXSkybox* skybox;
};
