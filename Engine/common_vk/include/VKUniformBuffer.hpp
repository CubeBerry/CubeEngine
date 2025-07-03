//Author: JEYOON YU
//Project: CubeEngine
//File: VKUniformBuffer.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include "VKInit.hpp"
#include "VKHelper.hpp"

template<typename Type>
class VKUniformBuffer
{
public:
	VKUniformBuffer(VKInit* init_, const int size_);
	~VKUniformBuffer();

	void InitUniformBuffer(const int count_);
	void UpdateUniform(size_t count_, void* data_, const uint32_t frameIndex_);

	std::array<VkBuffer, 2>* GetUniformBuffers() { return &vkUniformBuffers; }
	std::array<VkDeviceMemory, 2>* GetUniformDeviceMemories() { return &vkUniformDeviceMemories; }
	void* GetMappedMemory(const uint32_t frameIndex_) const
	{
		auto& vkUniformDeviceMemory = vkUniformDeviceMemories[frameIndex_];

		//Get Virtual Address for CPU to access Memory
		void* contents;
		VKHelper::ThrowIfFailed(vkMapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory, 0, sizeof(Type), 0, &contents));

		return contents;
	}
	void UnmapMemory(const uint32_t frameIndex_) const
	{
		auto& vkUniformDeviceMemory = vkUniformDeviceMemories[frameIndex_];
		vkUnmapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory);
	}
private:
	VKInit* vkInit;
	VkCommandPool* vkCommandPool;

	std::array<VkBuffer, 2> vkUniformBuffers{ VK_NULL_HANDLE };
	std::array<VkDeviceMemory, 2> vkUniformDeviceMemories{ VK_NULL_HANDLE };
};

template<typename Type>
VKUniformBuffer<Type>::VKUniformBuffer(VKInit* init_, const int size_) : vkInit(init_)
{
	InitUniformBuffer(size_);
}

template<typename Type>
VKUniformBuffer<Type>::~VKUniformBuffer()
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

template<typename Type>
inline void VKUniformBuffer<Type>::InitUniformBuffer(const int count_)
{
	for (auto i = 0; i != 2; ++i)
	{
		//Create Uniform Buffer Info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = sizeof(Type) * count_;
		createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		//Create Uniform Buffer
		VKHelper::ThrowIfFailed(vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkUniformBuffers[i]));

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements;
		//Get Memory Requirements for Buffer
		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkUniformBuffers[i], &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Can access from CPU and ensure memory sync between CPU and GPU
		allocateInfo.memoryTypeIndex = VKHelper::FindMemoryTypeIndex(*vkInit->GetPhysicalDevice(), requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Allocate Memory
		VKHelper::ThrowIfFailed(vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkUniformDeviceMemories[i]));

		//Bind Buffer and Memory
		VKHelper::ThrowIfFailed(vkBindBufferMemory(*vkInit->GetDevice(), vkUniformBuffers[i], vkUniformDeviceMemories[i], 0));
	}
}

template<typename Type>
inline void VKUniformBuffer<Type>::UpdateUniform(size_t count_, void* data_, const uint32_t frameIndex_)
{
	auto& vkUniformDeviceMemory = vkUniformDeviceMemories[frameIndex_];

	//Get Virtual Address for CPU to access Memory
	void* contents;
	VKHelper::ThrowIfFailed(vkMapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory, 0, sizeof(Type) * count_, 0, &contents));

	//auto material = static_cast<Type*>(contents);
	//*material = *material_;
	memcpy(contents, data_, sizeof(Type) * count_);

	vkUnmapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory);
}
