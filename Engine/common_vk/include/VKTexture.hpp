//Author: JEYOON YU
//Project: CubeEngine
//File: VKTexture.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <filesystem>

class VKInit;

class VKTexture
{
public:
	VKTexture(VKInit* init_, VkCommandPool* pool_);
	~VKTexture();

	void LoadTexture(const std::filesystem::path& path_);

	VkSampler* GetSampler() { return &vkTextureSampler; };
	VkImageView* GetImageView() { return &vkTextureImageView; };
	int GetWidth() const { return width; };
	int GetHeight() const { return height; };
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_);
	VKInit* vkInit;
	VkCommandPool* vkCommandPool{ VK_NULL_HANDLE };

	VkImage vkTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemory{ VK_NULL_HANDLE };
	VkImageView vkTextureImageView{ VK_NULL_HANDLE };
	VkSampler vkTextureSampler{ VK_NULL_HANDLE };

	int width, height;
};