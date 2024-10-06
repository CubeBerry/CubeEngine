//Author: JEYOON YU
//Project: CubeEngine
//File: VKUniformBuffer.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include "VKInit.hpp"
#include <iostream>

template<typename Material>
class VKUniformBuffer
{
public:
	VKUniformBuffer(VKInit* init_, const int size_);
	~VKUniformBuffer();

	void InitUniformBuffer(const int count_);
	void UpdateUniform(size_t count_, void* data_, const uint32_t frameIndex_);

	std::array<VkBuffer, 2>* GetUniformBuffers() { return &vkUniformBuffers; };
	std::array<VkDeviceMemory, 2>* GetUniformDeviceMemories() { return &vkUniformDeviceMemories; };
private:
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_);
	VKInit* vkInit;
	VkCommandPool* vkCommandPool;

	std::array<VkBuffer, 2> vkUniformBuffers{ VK_NULL_HANDLE };
	std::array<VkDeviceMemory, 2> vkUniformDeviceMemories{ VK_NULL_HANDLE };
};

template<typename Material>
VKUniformBuffer<Material>::VKUniformBuffer(VKInit* init_, const int size_) : vkInit(init_)
{
	InitUniformBuffer(size_);
}

template<typename Material>
VKUniformBuffer<Material>::~VKUniformBuffer()
{
	//Free Memories
	for (auto& memory : vkUniformDeviceMemories)
	{
		vkFreeMemory(*vkInit->GetDevice(), memory, nullptr);
	}
	//Destroy Uniform Buffers
	for (auto& buffer : vkUniformBuffers)
	{
		vkDestroyBuffer(*vkInit->GetDevice(), buffer, nullptr);
	}
}

template<typename Material>
inline void VKUniformBuffer<Material>::InitUniformBuffer(const int count_)
{
	for (auto i = 0; i != 2; ++i)
	{
		//Create Uniform Buffer Info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = sizeof(Material) * count_;
		createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		//Create Uniform Buffer
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkUniformBuffers[i]);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
					break;
				default:
					break;
				}
				std::cout << std::endl;

				throw std::runtime_error{ "Uniform Buffer Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKUniformBuffer::~VKUniformBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements;
		//Get Memory Requirements for Buffer
		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkUniformBuffers[i], &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Can access from CPU and ensure memory sync between CPU and GPU
		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Allocate Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkUniformDeviceMemories[i]);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
					break;
				default:
					break;
				}
				std::cout << std::endl;

				throw std::runtime_error{ "Uniform Memory Allocation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKUniformBuffer::~VKUniformBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Bind Buffer and Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkBindBufferMemory(*vkInit->GetDevice(), vkUniformBuffers[i], vkUniformDeviceMemories[i], 0);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
					break;
				default:
					break;
				}
				std::cout << std::endl;

				throw std::runtime_error{ "Memory Bind Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKUniformBuffer::~VKUniformBuffer();
			std::exit(EXIT_FAILURE);
		}
	}
}

template<typename Material>
inline void VKUniformBuffer<Material>::UpdateUniform(size_t count_, void* data_, const uint32_t frameIndex_)
{
	auto& vkUniformDeviceMemory = vkUniformDeviceMemories[frameIndex_];

	//Get Virtual Address for CPU to access Memory
	void* contents;
	vkMapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory, 0, sizeof(Material) * count_, 0, &contents);

	//auto material = static_cast<Material*>(contents);
	//*material = *material_;
	memcpy(contents, data_, sizeof(Material) * count_);

	vkUnmapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory);
}

template<typename Material>
inline uint32_t VKUniformBuffer<Material>::FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_)
{
	//Get Physical Device Memory Properties
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(*vkInit->GetPhysicalDevice(), &physicalDeviceMemoryProperties);

	//Find memory type index which satisfies both requirement and property
	for (uint32_t i = 0; i != physicalDeviceMemoryProperties.memoryTypeCount; ++i)
	{
		//Check if memory is allocatable at ith memory type
		if (!(requirements_.memoryTypeBits & (1 << i)))
			continue;

		//Check if satisfies memory property
		if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties_) != properties_)
			continue;

		return i;
	}
	return UINT32_MAX;
}
