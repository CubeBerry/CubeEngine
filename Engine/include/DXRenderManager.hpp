//Author: JEYOON YU
//Project: CubeEngine
//File: DXRenderManager.hpp
#pragma once
#include "RenderManager.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DXSkybox;

class DXRenderManager : public RenderManager
{
public:
	DXRenderManager() { gMode = GraphicsMode::DX; };
	~DXRenderManager();
	void Initialize(SDL_Window* window_);

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;
private:
	static const UINT frameCount = 2;

	// m = member
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[frameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	// rtv = Render Target View
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize{ 0 };

	UINT m_frameIndex{ 0 };
	HANDLE m_fenceEvent{ nullptr };
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue{ 0 };
public:
	//--------------------Common--------------------//
	void DeleteWithIndex(int id) override;

	void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
//		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().vertexBuffer = new DXVertexBuffer();
//#ifdef _DEBUG
//		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().normalVertexBuffer = new DXVertexBuffer();
//#endif
//		bufferWrapper.GetBuffer<BufferWrapper::DXBuffer>().indexBuffer = new DXIndexBuffer(&indices);
//		if (rMode == RenderType::TwoDimension)
//		{
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer2D>().vertexUniformBuffer = new DXUniformBuffer<TwoDimension::VertexUniform>();
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer2D>().fragmentUniformBuffer = new DXUniformBuffer<TwoDimension::FragmentUniform>();
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer2D>().vertexUniformBuffer->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", sizeof(TwoDimension::VertexUniform), nullptr);
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer2D>().fragmentUniformBuffer->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", sizeof(TwoDimension::FragmentUniform), nullptr);
//		}
//		else if (rMode == RenderType::ThreeDimension)
//		{
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer3D>().vertexUniformBuffer = new DXUniformBuffer<ThreeDimension::VertexUniform>();
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer3D>().fragmentUniformBuffer = new DXUniformBuffer<ThreeDimension::FragmentUniform>();
//
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer3D>().materialUniformBuffer = new DXUniformBuffer<ThreeDimension::Material>();
//
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer3D>().vertexUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 2, "vUniformMatrix", sizeof(ThreeDimension::VertexUniform), nullptr);
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer3D>().fragmentUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 3, "fUniformMatrix", sizeof(ThreeDimension::FragmentUniform), nullptr);
//			bufferWrapper.GetUniformBuffer<BufferWrapper::DXUniformBuffer3D>().materialUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 4, "fUniformMaterial", sizeof(ThreeDimension::Material), nullptr);
//		}
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
	//std::vector<DXTexture*> textures;
	std::vector<int> samplers;

#ifdef _DEBUG
	//DXVertexArray normalVertexArray;
	//DXShader glNormal3DShader;
#endif

	//Lighting
	//DXUniformBuffer<ThreeDimension::DirectionalLightUniform>* directionalLightUniformBuffer{ nullptr };
	//DXUniformBuffer<ThreeDimension::PointLightUniform>* pointLightUniformBuffer{ nullptr };

	//Skybox
	//DXVertexArray skyboxVertexArray;
	//DXVertexBuffer* skyboxVertexBuffer{ nullptr };
	//DXShader skyboxShader;
	//DXSkybox* skybox;
};
