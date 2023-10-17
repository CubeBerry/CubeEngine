#pragma once
#include <filesystem>
#include <vulkan/vulkan.hpp>

#include "VKTexture.hpp"
#include "glm/mat3x3.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"

class Texture
{
public:
	Texture(const std::filesystem::path& path_);
	~Texture();
	void Resize(glm::mat3 matrix_, const uint32_t frameIndex_);

	VkSampler* GetSampler() { return texture->GetSampler(); };
	VkImageView* GetImageView() { return texture->GetImageView(); };
	VkBuffer* GetVertexBuffer() { return vertex->GetVertexBuffer(); };
	VkBuffer* GetIndexBuffer() { return index->GetIndexBuffer(); };
	std::array<VkBuffer, 2>* GetUniformBuffers() { return uniform->GetUniformBuffers(); };
private:
	VKTexture* texture;
	VKVertexBuffer* vertex;
	VKIndexBuffer* index;
	VKUniformBuffer<glm::mat3>* uniform;

	glm::mat3 matrix;
};