//Author: JEYOON YU
//Project: CubeEngine
//File: VKTexture.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <glm/vec2.hpp>

class VKInit;

class VKTexture
{
public:
	VKTexture(VKInit* init_, VkCommandPool* pool_);
	~VKTexture();

	void LoadTexture(const std::filesystem::path& path_, std::string name_);
	void LoadSkyBox(
		const std::filesystem::path& right,
		const std::filesystem::path& left,
		const std::filesystem::path& top,
		const std::filesystem::path& bottom,
		const std::filesystem::path& front,
		const std::filesystem::path& back
	);
	void SetTextureID(int id) { texID = id; };

	VkSampler* GetSampler() { return &vkTextureSampler; };
	VkImageView* GetImageView() { return &vkTextureImageView; };
	int GetWidth() const { return width; };
	int GetHeight() const { return height; };
	glm::vec2 GetSize() const { return glm::vec2(width, height); };
	std::string GetName() const { return name; };
	int GetTextrueId() { return texID; }
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_);
	VKInit* vkInit;
	VkCommandPool* vkCommandPool{ VK_NULL_HANDLE };

	VkImage vkTextureImage{ VK_NULL_HANDLE };
	VkDeviceMemory vkTextureDeviceMemory{ VK_NULL_HANDLE };
	VkImageView vkTextureImageView{ VK_NULL_HANDLE };
	VkSampler vkTextureSampler{ VK_NULL_HANDLE };

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