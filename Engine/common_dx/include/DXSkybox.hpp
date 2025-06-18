//Author: JEYOON YU
//Project: CubeEngine
//File: DXSkybox.hpp
#pragma once
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

class DXTexture;

class DXSkybox
{
public:
	DXSkybox(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::filesystem::path& path);
	~DXSkybox();
	void ExecuteCommandList();

	void EquirectangularToCube();
	void CalculateIrradiance();
	void PrefilteredEnvironmentMap();
	void BRDFLUT();

	//std::pair<VkSampler*, VkImageView*> GetCubeMap() { return { &vkTextureSamplerEquirectangular, &vkTextureImageViewEquirectangular }; }
	//std::pair<VkSampler*, VkImageView*> GetIrradiance() { return { &vkTextureSamplerIrradiance, &vkTextureImageViewIrradiance }; }
	//std::pair<VkSampler*, VkImageView*> GetPrefilter() { return { &vkTextureSamplerPrefilter, &vkTextureImageViewPrefilter }; }
	//std::pair<VkSampler*, VkImageView*> GetBRDF() { return { &vkTextureSamplerBRDFLUT, &vkTextureImageViewBRDFLUT }; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetIrradianceMapSrv() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetPrefilterMapSrv() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetBrdfLutSrv() const;
private:
	std::unique_ptr<DXTexture> m_equirectangularMap;

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue{ 1 };

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescriptorSize;

	ComPtr<ID3D12Resource> m_cubemap;
	ComPtr<ID3D12Resource> m_irradianceMap;
	ComPtr<ID3D12Resource> m_prefilterMap;
	ComPtr<ID3D12Resource> m_brdfLUT;

	//Equirectangular to Cube
	uint32_t faceSize{ 0 };
	//Irradiance
	uint32_t irradianceSize{ 64 };
	//Prefilter
	uint32_t baseSize{ 512 };
	uint32_t mipLevels{ 5 };
	//BRDF LUT
	uint32_t lutSize{ 512 };

	//CubeMap converted from Equirectangular
	//VkImage vkTextureImageEquirectangular{ DX_NULL_HANDLE };
	//VkDeviceMemory vkTextureDeviceMemoryEquirectangular{ DX_NULL_HANDLE };
	//VkImageView vkTextureImageViewEquirectangular{ DX_NULL_HANDLE };
	//VkSampler vkTextureSamplerEquirectangular{ DX_NULL_HANDLE };

	//Irradiance Texture
	//VkImage vkTextureImageIrradiance{ DX_NULL_HANDLE };
	//VkDeviceMemory vkTextureDeviceMemoryIrradiance{ DX_NULL_HANDLE };
	//VkImageView vkTextureImageViewIrradiance{ DX_NULL_HANDLE };
	//VkSampler vkTextureSamplerIrradiance{ DX_NULL_HANDLE };

	//Prefilter Texture
	//VkImage vkTextureImagePrefilter{ DX_NULL_HANDLE };
	//VkDeviceMemory vkTextureDeviceMemoryPrefilter{ DX_NULL_HANDLE };
	//VkImageView vkTextureImageViewPrefilter{ DX_NULL_HANDLE };
	//VkSampler vkTextureSamplerPrefilter{ DX_NULL_HANDLE };

	//BRDF LUT Texture
	//VkImage vkTextureImageBRDFLUT{ DX_NULL_HANDLE };
	//VkDeviceMemory vkTextureDeviceMemoryBRDFLUT{ DX_NULL_HANDLE };
	//VkImageView vkTextureImageViewBRDFLUT{ DX_NULL_HANDLE };
	//VkSampler vkTextureSamplerBRDFLUT{ DX_NULL_HANDLE };

	std::vector<glm::vec3> m_skyboxVertices = {
		{-1.0f, -1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},

		{-1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},

		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},

		{-1.0f,  1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},

		{-1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f, -1.0f},

		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f}
	};
	//BRDF LUT fullscreen quad texture
	std::vector<glm::vec3> m_fullscreenQuad = {
	glm::vec3(-1.0f, -1.0f, 0.0f),
	glm::vec3(1.0f, -1.0f, 0.0f),
	glm::vec3(-1.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 0.0f),
	};
	std::vector<glm::vec2> m_fullscreenQuadTexCoords = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f),
	};

	const float clearColor[4] = { 0.f, 0.f, 0.f, 1.0f };
	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	glm::mat4 views[6] = {
		glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f,0.f, 1.f)),
		glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f,0.f, -1.f)),
		glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
	};
};