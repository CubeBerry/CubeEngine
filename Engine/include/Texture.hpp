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
	Texture(const std::filesystem::path& path_, int index_);
	~Texture();
	void Resize(UniformMatrix* matrix_, const uint32_t frameIndex_);

	const glm::vec2 GetImageSize() { return glm::vec2(texture->GetWidth(), texture->GetHeight()); };
	VkSampler* GetSampler() { return texture->GetSampler(); };
	VkImageView* GetImageView() { return texture->GetImageView(); };
	VkBuffer* GetVertexBuffer() { return vertex->GetVertexBuffer(); };
	VkBuffer* GetIndexBuffer() { return index->GetIndexBuffer(); };
	std::array<VkBuffer, 2>* GetUniformBuffers() { return uniform->GetUniformBuffers(); };
	std::array<VkDeviceMemory, 2>* GetUniformDeviceMemories() { return uniform->GetUniformDeviceMemories(); };
private:
	VKTexture* texture;
	VKVertexBuffer* vertex;
	VKIndexBuffer* index;
	VKUniformBuffer<UniformMatrix>* uniform;

	UniformMatrix* matrix;
};