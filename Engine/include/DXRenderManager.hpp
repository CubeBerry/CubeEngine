//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.hpp
#pragma once
#include "RenderManager.hpp"

#include <directx/d3dx12.h>
#include <dxgi1_6.h>

#include "DXPipeLine.hpp"

// @TODO temporal forward declaration classes for DirectX
class DXTexture;
class DXSkybox;
class DXShader;

using Microsoft::WRL::ComPtr;

class DXSkybox;

class DXRenderManager : public RenderManager
{
public:
	DXRenderManager() { gMode = GraphicsMode::DX; }
	~DXRenderManager();
	void Initialize(SDL_Window* window_);

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;
private:
	// Initialize DirectX 12 components
	void CreateRootSignature();
	void WaitForGPU();

	static const UINT frameCount = 2;

	// m = member
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[frameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[frameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	// rtv = Render Target View
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// dsv = Depth Stencil View
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	// cbv/srv = Constant Buffer View / Shader Resource View
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_depthStencil;

	UINT m_rtvDescriptorSize{ 0 };
	UINT m_cbvSrvDescriptorSize{ 0 };

	UINT m_frameIndex{ 0 };
	HANDLE m_fenceEvent{ nullptr };
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[frameCount] = {};

	std::unique_ptr<DXPipeLine> m_pipeline2D;
public:
	//--------------------Common--------------------//
	void DeleteWithIndex(int id) override;

	void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().indexBuffer = new DXIndexBuffer(m_device, &indices);
		if (rMode == RenderType::TwoDimension)
		{
			auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = new DXVertexBuffer(m_device, sizeof(TwoDimension::Vertex), static_cast<UINT>(vertices.size()), vertices.data());

			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>().vertexUniformBuffer = new DXConstantBuffer<TwoDimension::VertexUniform>(m_device);
			bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer2D>().fragmentUniformBuffer = new DXConstantBuffer<TwoDimension::FragmentUniform>(m_device);
		}
		else if (rMode == RenderType::ThreeDimension)
		{
			auto& vertices = bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertices;
			bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = new DXVertexBuffer(m_device, sizeof(TwoDimension::Vertex), static_cast<UINT>(vertices.size()), vertices.data());
#ifdef _DEBUG
			//bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().normalVertexBuffer = new DXVertexBuffer();
#endif

			//bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().vertexUniformBuffer = new DXConstantBuffer<ThreeDimension::VertexUniform>();
			//bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().fragmentUniformBuffer = new DXConstantBuffer<ThreeDimension::FragmentUniform>();
			//bufferWrapper.GetUniformBuffer<BufferWrapper::DXConstantBuffer3D>().materialUniformBuffer = new DXConstantBuffer<ThreeDimension::Material>();
		}
	}

	//--------------------2D Render--------------------//
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;

	//DXTexture* GetTexture(std::string name);
	//std::vector<DXTexture*> GetTextures() { return textures; }

	//--------------------3D Render--------------------//

	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
private:
	//--------------------Common--------------------//
	std::vector<DXTexture*> textures;

#ifdef _DEBUG
	//DXShader glNormal3DShader;
#endif

	//Lighting
	DXConstantBuffer<ThreeDimension::DirectionalLightUniform>* directionalLightUniformBuffer{ nullptr };
	DXConstantBuffer<ThreeDimension::PointLightUniform>* pointLightUniformBuffer{ nullptr };

	//Skybox
	DXVertexBuffer* skyboxVertexBuffer{ nullptr };
	//DXShader skyboxShader;
	//DXSkybox* skybox;
};
