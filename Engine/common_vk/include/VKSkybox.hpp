//Author: JEYOON YU
//Project: CubeEngine
//File: VKSkybox.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

class VKInit;
class VKTexture;

class VKSkybox
{
public:
	VKSkybox(const std::filesystem::path& path, VKInit* init_, VkCommandPool* pool_);
	~VKSkybox();

	void EquirectangularToCube(VkCommandBuffer* commandBuffer);
	void CalculateIrradiance(VkCommandBuffer* commandBuffer);
	void PrefilteredEnvironmentMap(VkCommandBuffer* commandBuffer);
	void BRDFLUT(VkCommandBuffer* commandBuffer);

	std::pair<VkSampler*, VkImageView*> GetCubeMap() { return { &vkTextureSamplerEquirectangular, &vkTextureImageViewEquirectangular }; };
	std::pair<VkSampler*, VkImageView*> GetIrradiance() { return { &vkTextureSamplerIrradiance, &vkTextureImageViewIrradiance }; };
	std::pair<VkSampler*, VkImageView*> GetPrefilter() { return { &vkTextureSamplerPrefilter, &vkTextureImageViewPrefilter }; };
	std::pair<VkSampler*, VkImageView*> GetBRDF() { return { &vkTextureSamplerBRDFLUT, &vkTextureImageViewBRDFLUT }; };
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_);
	VKInit* vkInit;
	VkCommandPool* vkCommandPool{ VK_NULL_HANDLE };
	VkCommandBuffer skyboxCommandBuffer{ VK_NULL_HANDLE };
	VKTexture* skyboxTexture;
	VkRenderPass renderPassIBL;
	std::array<VkImageView, 6> cubeFaceViews;
	std::array<VkFramebuffer, 6> cubeFaceFramebuffers;

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
	VkImage vkTextureImageEquirectangular{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemoryEquirectangular{ VK_NULL_HANDLE };
	VkImageView vkTextureImageViewEquirectangular{ VK_NULL_HANDLE };
	VkSampler vkTextureSamplerEquirectangular{ VK_NULL_HANDLE };

	//Irradiance Texture
	VkImage vkTextureImageIrradiance{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemoryIrradiance{ VK_NULL_HANDLE };
	VkImageView vkTextureImageViewIrradiance{ VK_NULL_HANDLE };
	VkSampler vkTextureSamplerIrradiance{ VK_NULL_HANDLE };

	//Prefilter Texture
	VkImage vkTextureImagePrefilter{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemoryPrefilter{ VK_NULL_HANDLE };
	VkImageView vkTextureImageViewPrefilter{ VK_NULL_HANDLE };
	VkSampler vkTextureSamplerPrefilter{ VK_NULL_HANDLE };

	//BRDF LUT Texture
	VkImage vkTextureImageBRDFLUT{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemoryBRDFLUT{ VK_NULL_HANDLE };
	VkImageView vkTextureImageViewBRDFLUT{ VK_NULL_HANDLE };
	VkSampler vkTextureSamplerBRDFLUT{ VK_NULL_HANDLE };

	std::vector<glm::vec3> skyboxVertices = {
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{1.0f, -1.0f, -1.0f },
		{1.0f, -1.0f, -1.0f},
		{1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		
		{-1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},
	
		{1.0f, -1.0f, -1.0f},
		{1.0f, -1.0f,  1.0f},
		{1.0f,  1.0f,  1.0f},
		{1.0f,  1.0f,  1.0f},
		{1.0f,  1.0f, -1.0f},
		{1.0f, -1.0f, -1.0f},
		
		{-1.0f, -1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},
		
		{-1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},
		
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},	
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f}
	};
	//BRDF LUT fullscreen quad texture
	std::vector<glm::vec3> fullscreenQuad = {
	glm::vec3(-1.0f, -1.0f, 0.0f),
	glm::vec3(-1.0f, 1.0f, 0.0f),
	glm::vec3(1.0f, -1.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 0.0f),
	};
	std::vector<glm::vec2> fullscreenQuadTexCoords = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
	};

	VkClearValue clearColor = { 0.f,0.f,0.f,1.f };
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