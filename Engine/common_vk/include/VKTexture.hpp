//Author: JEYOON YU
//Project: CubeEngine
//File: VKTexture.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class VKInit;

class VKTexture
{
public:
	VKTexture(VKInit* init_, VkCommandPool* pool_);
	~VKTexture();

	void LoadTexture(bool isHDR, const std::filesystem::path& path_, std::string name_, bool flip);
	void LoadSkyBox(
		bool isHDR,
		const std::filesystem::path& right,
		const std::filesystem::path& left,
		const std::filesystem::path& top,
		const std::filesystem::path& bottom,
		const std::filesystem::path& front,
		const std::filesystem::path& back
	);
	void EquirectangularToCube(VkCommandBuffer* commandBuffer);
	void CalculateIrradiance(VkCommandBuffer* commandBuffer);
	void PrefilteredEnvironmentMap(VkCommandBuffer* commandBuffer);
	void BRDFLUT(VkCommandBuffer* commandBuffer);
	void SetTextureID(int id) { texID = id; };

	VkSampler* GetSampler()
	{
		if (isEquirectangular) return &vkTextureSamplerEquirectangular;
		return &vkTextureSampler;
	};
	VkImageView* GetImageView()
	{
		if (isEquirectangular) return &vkTextureImageViewEquirectangular;
		return &vkTextureImageView;
	};
	std::pair<VkSampler*, VkImageView*> GetIrradiance() { return { &vkTextureSamplerIrradiance, &vkTextureImageViewIrradiance }; };
	std::pair<VkSampler*, VkImageView*> GetPrefilter() { return { &vkTextureSamplerPrefilter, &vkTextureImageViewPrefilter }; };
	std::pair<VkSampler*, VkImageView*> GetBRDF() { return { &vkTextureSamplerBRDFLUT, &vkTextureImageViewBRDFLUT }; };
	//VkSampler* GetSamplerIBL() { return &vkTextureSamplerIBL; };
	//VkImageView* GetImageViewIBL() { return &vkTextureImageViewIBL; };

	bool GetIsEquirectangular() { return isEquirectangular; };

	int GetWidth() const { return width; };
	int GetHeight() const { return height; };
	glm::vec2 GetSize() const { return glm::vec2(width, height); };
	std::string GetName() const { return name; };
	int GetTextrueId() { return texID; };
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_);
	VKInit* vkInit;
	VkCommandPool* vkCommandPool{ VK_NULL_HANDLE };

	//2D Texture & CubeMap for Rendering
	VkImage vkTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemory{ VK_NULL_HANDLE };
	VkImageView vkTextureImageView{ VK_NULL_HANDLE };
	VkSampler vkTextureSampler{ VK_NULL_HANDLE };

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

	std::array<VkImageView, 6> cubeFaceViews;
	VkRenderPass renderPassIBL;
	std::array<VkFramebuffer, 6> cubeFaceFramebuffers;
	uint32_t faceSize{ 0 };
	bool isEquirectangular{ false };

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
	std::vector<glm::vec3> fullscreenQuad = {
		glm::vec3(-1.0f, 1.0f, 0.0f),
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
	};
	std::vector<glm::vec2> fullscreenQuadTexCoords = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
	};

	int width, height;
	int texID;
	std::string name;

	void FlipTextureHorizontally(uint8_t* src, int width_, int height_, int numComponents) const
	{
		for (int row = 0; row < height_; ++row)
		{
			for (int column = 0; column < width_ / 2; ++column)
			{
				int src_row{ row };
				int src_column{ column };
				int dest_column = width_ - 1 - src_column;

				int src_index{ (src_row * width_ + src_column) * numComponents };
				int dest_index{ (src_row * width_ + dest_column) * numComponents };

				for (int component = 0; component < numComponents; ++component)
					std::swap(src[src_index + component], src[dest_index + component]);
			}
		}
	};
};