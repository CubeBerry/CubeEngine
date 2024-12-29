//Author: JEYOON YU
//Project: CubeEngine
//File: VKIndexBuffer.cpp
#include "VKIndexBuffer.hpp"
#include "VKInit.hpp"
#include <iostream>

VKIndexBuffer::VKIndexBuffer(VKInit* init_, VkCommandPool* pool_, std::vector<uint32_t>* indices_) : vkInit(init_), vkCommandPool(pool_)
{
	InitIndexBuffer(indices_);
}

VKIndexBuffer::~VKIndexBuffer()
{
	//Free Memory
	vkFreeMemory(*vkInit->GetDevice(), vkIndexDeviceMemory, nullptr);
	//Destroy Vertex Buffer
	vkDestroyBuffer(*vkInit->GetDevice(), vkIndexBuffer, nullptr);
}

uint32_t VKIndexBuffer::FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_)
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

void VKIndexBuffer::InitIndexBuffer(std::vector<uint32_t>* indices_)
{
	{
		//Create Index Buffer Info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = sizeof(uint32_t) * indices_->size();
		createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		//Create Vertex Buffer
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkIndexBuffer);
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

				throw std::runtime_error{ "Index Buffer Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements;
		//Get Memory Requirements for Buffer
		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkIndexBuffer, &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Fast GPU access but CPU cannot access
		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//Allocate Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkIndexDeviceMemory);
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

				throw std::runtime_error{ "Index Memory Allocation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Bind Buffer and Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkBindBufferMemory(*vkInit->GetDevice(), vkIndexBuffer, vkIndexDeviceMemory, 0);
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
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}
	}

	//--------------------Staging Buffer--------------------//

	VkBuffer vkStagingBuffer;
	VkDeviceMemory vkStagingDeviceMemory;
	{
		//Create Staging Buffer Info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = sizeof(uint32_t) * indices_->size();
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		//Create Staging Buffer
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkStagingBuffer);
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

				throw std::runtime_error{ "Staging Buffer Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements;
		//Get Memory Requirements for Buffer
		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkStagingBuffer, &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Select memory type which CPU can access and ensures memory sync between CPU and GPU
		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Allocate Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkStagingDeviceMemory);
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

				throw std::runtime_error{ "Staging Memory Allocation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Bind Buffer and Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkBindBufferMemory(*vkInit->GetDevice(), vkStagingBuffer, vkStagingDeviceMemory, 0);
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
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Get Virtual Address for CPU to access Memory
		void* contents;
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkMapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, 0, sizeof(uint32_t) * indices_->size(), 0, &contents);
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
				case VK_ERROR_MEMORY_MAP_FAILED:
					std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
					break;
				default:
					break;
				}
				std::cout << std::endl;

				throw std::runtime_error{ "Memory Map Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Copy Vertex Info to Memory
		//&(*indices_)[0] == vertices_->data()
		memcpy(contents, indices_->data(), sizeof(uint32_t)* indices_->size());

		//End Accessing Memory from CPU
		vkUnmapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory);
	}

	//Create Command Buffer Allocate Info
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = *vkCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	//Create Command Buffer
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &commandBuffer);

	//Create Command Buffer Begin Info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin Command Buffer
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	//Set Copy Region
	VkBufferCopy region{};
	region.size = sizeof(uint32_t) * indices_->size();

	//Copy Staging Buffer to Index Buffer as Designated Region
	vkCmdCopyBuffer(commandBuffer, vkStagingBuffer, vkIndexBuffer, 1, &region);

	//End Command Buffer
	vkEndCommandBuffer(commandBuffer);

	//Create Submit Info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//Submit Queue to Command Buffer
	//vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *vkSwapChain->GetFence());
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//Deallocate Command Buffers
	vkFreeCommandBuffers(*vkInit->GetDevice(), *vkCommandPool, 1, &commandBuffer);

	//Free Staging Device Memory
	vkFreeMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, nullptr);

	//Destroy Staging Buffer
	vkDestroyBuffer(*vkInit->GetDevice(), vkStagingBuffer, nullptr);
}

void VKIndexBuffer::UpdateIndexBuffer(std::vector<uint32_t>* indices_)
{
	VkBuffer vkStagingBuffer;
	VkDeviceMemory vkStagingDeviceMemory;
	{
		//Create Staging Buffer Info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = sizeof(uint32_t) * indices_->size();
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		//Create Staging Buffer
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkStagingBuffer);
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

				throw std::runtime_error{ "Staging Buffer Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements;
		//Get Memory Requirements for Buffer
		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkStagingBuffer, &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Select memory type which CPU can access and ensures memory sync between CPU and GPU
		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Allocate Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkStagingDeviceMemory);
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

				throw std::runtime_error{ "Staging Memory Allocation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Bind Buffer and Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkBindBufferMemory(*vkInit->GetDevice(), vkStagingBuffer, vkStagingDeviceMemory, 0);
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
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Get Virtual Address for CPU to access Memory
		void* contents;
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkMapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, 0, sizeof(uint32_t) * indices_->size(), 0, &contents);
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
				case VK_ERROR_MEMORY_MAP_FAILED:
					std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
					break;
				default:
					break;
				}
				std::cout << std::endl;

				throw std::runtime_error{ "Memory Map Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKIndexBuffer::~VKIndexBuffer();
			std::exit(EXIT_FAILURE);
		}

		//Copy Vertex Info to Memory
		//&(*indices_)[0] == vertices_->data()
		memcpy(contents, indices_->data(), sizeof(uint32_t) * indices_->size());

		//End Accessing Memory from CPU
		vkUnmapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory);
	}

	//Create Command Buffer Allocate Info
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = *vkCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	//Create Command Buffer
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &commandBuffer);

	//Create Command Buffer Begin Info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin Command Buffer
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	//Set Copy Region
	VkBufferCopy region{};
	region.size = sizeof(uint32_t) * indices_->size();

	//Copy Staging Buffer to Index Buffer as Designated Region
	vkCmdCopyBuffer(commandBuffer, vkStagingBuffer, vkIndexBuffer, 1, &region);

	//End Command Buffer
	vkEndCommandBuffer(commandBuffer);

	//Create Submit Info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//Submit Queue to Command Buffer
	//vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *vkSwapChain->GetFence());
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//Deallocate Command Buffers
	vkFreeCommandBuffers(*vkInit->GetDevice(), *vkCommandPool, 1, &commandBuffer);

	//Free Staging Device Memory
	vkFreeMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, nullptr);

	//Destroy Staging Buffer
	vkDestroyBuffer(*vkInit->GetDevice(), vkStagingBuffer, nullptr);
}
