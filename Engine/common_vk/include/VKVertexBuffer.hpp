#pragma once
#include <vulkan/vulkan.hpp>
#include "Vertex.hpp"

class VKInit;

class VKVertexBuffer
{
public:
	VKVertexBuffer(VKInit* init_, std::vector<Vertex>* vertices_);
	~VKVertexBuffer();

	void InitVertexBuffer(std::vector<Vertex>* vertices_);
	VkBuffer* GetVertexBuffer() { return &vkVertexBuffer; };
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_);
	VKInit* vkInit;

	VkBuffer vkVertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory vkVertexDeviceMemory{ VK_NULL_HANDLE };
};