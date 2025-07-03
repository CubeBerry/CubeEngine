//Author: JEYOON YU
//Project: CubeEngine
//File: VKVertexBuffer.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>

#include "VKInit.hpp"
#include "VKHelper.hpp"

class VKVertexBuffer
{
public:
	VKVertexBuffer(VKInit* init_, VkDeviceSize size_, const void* data) : vkInit(init_)
	{
		InitVertexBuffer(size_, data);
	}

	~VKVertexBuffer()
	{
		// Free Memory
		vkFreeMemory(*vkInit->GetDevice(), vkVertexDeviceMemory, nullptr);
		// Destroy Vertex Buffer
		vkDestroyBuffer(*vkInit->GetDevice(), vkVertexBuffer, nullptr);
	}

	void InitVertexBuffer(VkDeviceSize size_, const void* data)
	{
		// Create Vertex Buffer Info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size_;
		createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// Create Vertex Buffer
		VKHelper::ThrowIfFailed(vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkVertexBuffer));

		// Declare a variable which will take memory requirements
		VkMemoryRequirements requirements;
		// Get Memory Requirements for Buffer
		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkVertexBuffer, &requirements);

		// Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		// Select memory type which CPU can access and ensures memory sync between CPU and GPU
		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Allocate Memory
		VKHelper::ThrowIfFailed(vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkVertexDeviceMemory));

		// Bind Buffer and Memory
		VKHelper::ThrowIfFailed(vkBindBufferMemory(*vkInit->GetDevice(), vkVertexBuffer, vkVertexDeviceMemory, 0));

		// Get Virtual Address for CPU to access Memory
		void* contents;
		VKHelper::ThrowIfFailed(vkMapMemory(*vkInit->GetDevice(), vkVertexDeviceMemory, 0, size_, 0, &contents));

		// Copy Vertex Info to Memory
		memcpy(contents, data, size_);

		// End Accessing Memory from CPU
		vkUnmapMemory(*vkInit->GetDevice(), vkVertexDeviceMemory);
	}

	void UpdateVertexBuffer(VkDeviceSize size_, void* data)
	{
		// Get Virtual Address for CPU to access Memory
		void* contents;
		VKHelper::ThrowIfFailed(vkMapMemory(*vkInit->GetDevice(), vkVertexDeviceMemory, 0, size_, 0, &contents));

		// Copy Vertex Info to Memory
		memcpy(contents, data, size_);

		// End Accessing Memory from CPU
		vkUnmapMemory(*vkInit->GetDevice(), vkVertexDeviceMemory);
	}

	VkBuffer* GetVertexBuffer() { return &vkVertexBuffer; }
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_)
	{
		// Get Physical Device Memory Properties
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(*vkInit->GetPhysicalDevice(), &physicalDeviceMemoryProperties);

		// Find memory type index which satisfies both requirement and property
		for (uint32_t i = 0; i != physicalDeviceMemoryProperties.memoryTypeCount; ++i)
		{
			// Check if memory is allocatable at ith memory type
			if (!(requirements_.memoryTypeBits & (1 << i)))
				continue;

			// Check if satisfies memory property
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties_) != properties_)
				continue;

			return i;
		}
		return UINT32_MAX;
	}
	VKInit* vkInit;

	VkBuffer vkVertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory vkVertexDeviceMemory{ VK_NULL_HANDLE };
};