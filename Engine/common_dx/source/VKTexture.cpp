////Author: JEYOON YU
////Project: CubeEngine
////File: VKTexture.cpp
//#include "VKTexture.hpp"
//#include "VKInit.hpp"
//#include "VKDescriptor.hpp"
//#include "VKShader.hpp"
//#include <iostream>
//
//#define STB_IMAGE_IMPLEMENTATION
//#pragma warning(push, 0)
//#include "stb-master/stb_image.h"
//#pragma warning(pop)
//
//VKTexture::VKTexture(VKInit* init_, VkCommandPool* pool_) : vkInit(init_), vkCommandPool(pool_)
//{
//}
//
//VKTexture::~VKTexture()
//{
//	//Destroy Sampler
//	vkDestroySampler(*vkInit->GetDevice(), vkTextureSampler, nullptr);
//	//Destroy ImageView
//	vkDestroyImageView(*vkInit->GetDevice(), vkTextureImageView, nullptr);
//	//Free Memory
//	vkFreeMemory(*vkInit->GetDevice(), vkTextureDeviceMemory, nullptr);
//	//Destroy Image
//	vkDestroyImage(*vkInit->GetDevice(), vkTextureImage, nullptr);
//}
//
//uint32_t VKTexture::FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_)
//{
//	//Get Physical Device Memory Properties
//	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
//	vkGetPhysicalDeviceMemoryProperties(*vkInit->GetPhysicalDevice(), &physicalDeviceMemoryProperties);
//
//	//Find memory type index which satisfies both requirement and property
//	for (uint32_t i = 0; i != physicalDeviceMemoryProperties.memoryTypeCount; ++i)
//	{
//		//Check if memory is allocatable at ith memory type
//		if (!(requirements_.memoryTypeBits & (1 << i)))
//			continue;
//
//		//Check if satisfies memory property
//		if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties_) != properties_)
//			continue;
//
//		return i;
//	}
//	return UINT32_MAX;
//}
//
//void VKTexture::LoadTexture(bool isHDR, const std::filesystem::path& path_, std::string name_, bool flip)
//{
//	name = name_;
//
//	if (flip) stbi_set_flip_vertically_on_load(true);
//
//	auto path = path_;
//	int texChannels;
//	//Read in image file
//	void* data{ nullptr };
//	if (isHDR)
//	{
//		data = stbi_loadf(path.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//	}
//	else
//		data = stbi_load(path.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//
//	{
//		//Define an image to create
//		VkImageCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//		createInfo.imageType = VK_IMAGE_TYPE_2D;
//		if (isHDR) createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//		else  createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//		createInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
//		createInfo.mipLevels = 1;
//		createInfo.arrayLayers = 1;
//		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//		//Use Optimal Tiling to make GPU effectively process image
//		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//		//Usage for copying and shader
//		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//
//		//Create image
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImage);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Image Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Declare a variable which will take memory requirements
//		VkMemoryRequirements requirements{};
//		//Get Memory Requirements for Image
//		vkGetImageMemoryRequirements(*vkInit->GetDevice(), vkTextureImage, &requirements);
//
//		//Create Memory Allocation Info
//		VkMemoryAllocateInfo allocateInfo{};
//		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		allocateInfo.allocationSize = requirements.size;
//		//Select memory type which has fast access from GPU
//		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//
//		//Allocate Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkTextureDeviceMemory);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_TOO_MANY_OBJECTS:
//					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Texture Memory Allocation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Bind Image and Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkBindImageMemory(*vkInit->GetDevice(), vkTextureImage, vkTextureDeviceMemory, 0);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Memory Bind Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//	}
//
//	//--------------------Staging Buffer--------------------//
//
//	VkBuffer vkStagingBuffer;
//	VkDeviceMemory vkStagingDeviceMemory;
//	VkDeviceSize imageSize{ 0 };
//	if (isHDR) imageSize = width * height * STBI_rgb_alpha * sizeof(float);
//	else imageSize = width * height * STBI_rgb_alpha;
//	{
//		//Create Staging Buffer Info
//		VkBufferCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//		createInfo.size = imageSize;
//		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
//		//Create Staging Buffer
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkStagingBuffer);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Staging Buffer Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Declare a variable which will take memory requirements
//		VkMemoryRequirements requirements;
//		//Get Memory Requirements for Buffer
//		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkStagingBuffer, &requirements);
//
//		//Create Memory Allocation Info
//		VkMemoryAllocateInfo allocateInfo{};
//		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		allocateInfo.allocationSize = requirements.size;
//		//Select memory type which CPU can access and ensures memory sync between CPU and GPU
//		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//		//Allocate Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkStagingDeviceMemory);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_TOO_MANY_OBJECTS:
//					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Staging Memory Allocation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Bind Buffer and Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkBindBufferMemory(*vkInit->GetDevice(), vkStagingBuffer, vkStagingDeviceMemory, 0);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Memory Bind Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Get Virtual Address for CPU to access Memory
//		void* contents;
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkMapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, 0, imageSize, 0, &contents);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_MEMORY_MAP_FAILED:
//					std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Memory Map Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Copy Bitmap Info to Memory
//		memcpy(contents, data, imageSize);
//
//		//End Accessing Memory from CPU
//		vkUnmapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory);
//	}
//
//	//Deallocate memory for reading image
//	stbi_image_free(data);
//
//	//Image is in staging buffer so that processing which copies staging buffer's content to image is needed
//	{
//		//Create CommandBuffer Allocate Info
//		VkCommandBufferAllocateInfo allocateInfo{};
//		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//		allocateInfo.commandPool = *vkCommandPool;
//		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//		allocateInfo.commandBufferCount = 1;
//
//		//Create CommandBuffer
//		VkCommandBuffer commandBuffer;
//		vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &commandBuffer);
//
//		//Create CommandBuffer Begin Info
//		VkCommandBufferBeginInfo beginInfo{};
//		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//		//Begin CommandBuffer
//		vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//		{
//			//Create Image Memory Barrier
//			//Layout should be TRANSFER_DST_OPTIMAL to do copy
//			VkImageMemoryBarrier barrier{};
//			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//			barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.image = vkTextureImage;
//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			barrier.subresourceRange.levelCount = 1;
//			barrier.subresourceRange.layerCount = 1;
//
//			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//		}
//
//		//Define Image Subresource which will be copied
//		VkImageSubresourceLayers subresourceLayers{};
//		subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		subresourceLayers.mipLevel = 0;
//		subresourceLayers.layerCount = 1;
//
//		//Define Copy Region
//		VkBufferImageCopy region{};
//
//		//It assumes region has same size as image if buffer region is not defined
//		region.imageSubresource = subresourceLayers;
//		region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
//
//		//Copy Staging Buffer to Image by defined region
//		vkCmdCopyBufferToImage(commandBuffer, vkStagingBuffer, vkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
//
//		{
//			//Create Image Memory Barrier
//			//Layout should be SHADER_READ_ONLY_OPTIMAL to be used at Shader
//			VkImageMemoryBarrier barrier{};
//			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//			barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.image = vkTextureImage;
//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			barrier.subresourceRange.levelCount = 1;
//			barrier.subresourceRange.layerCount = 1;
//
//			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//		}
//
//		//End CommandBuffer
//		vkEndCommandBuffer(commandBuffer);
//
//		//Create submit info
//		VkSubmitInfo submitInfo{};
//		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//		submitInfo.commandBufferCount = 1;
//		submitInfo.pCommandBuffers = &commandBuffer;
//
//		//Submit queue to command buffer
//		vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);
//
//		//Wait until all submitted command buffers are handled
//		vkDeviceWaitIdle(*vkInit->GetDevice());
//
//		//Free CommandBuffer
//		vkFreeCommandBuffers(*vkInit->GetDevice(), *vkCommandPool, 1, &commandBuffer);
//
//		//Free Memory
//		vkFreeMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, nullptr);
//
//		//Destroy Staging Buffer
//		vkDestroyBuffer(*vkInit->GetDevice(), vkStagingBuffer, nullptr);
//	}
//
//	{
//		//To access image from graphics pipeline, Image View is needed
//		//Create ImageView Info
//		VkImageViewCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		createInfo.image = vkTextureImage;
//		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		if (isHDR) createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//		else  createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		createInfo.subresourceRange.levelCount = 1;
//		createInfo.subresourceRange.layerCount = 1;
//
//		//Create ImageView
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageView);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Image View Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//	}
//
//	{
//		//Sampler is needed for shader to read image
//		//Create Sampler Info
//		VkSamplerCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//		if (isHDR)
//		{
//			createInfo.magFilter = VK_FILTER_LINEAR;
//			createInfo.minFilter = VK_FILTER_LINEAR;
//		}
//		else
//		{
//			createInfo.magFilter = VK_FILTER_NEAREST;
//			createInfo.minFilter = VK_FILTER_NEAREST;
//		}
//		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//		createInfo.unnormalizedCoordinates = VK_FALSE;
//
//		//Create Sampler
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureSampler);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_TOO_MANY_OBJECTS:
//					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Image Sampler Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//	}
//}
//
//void VKTexture::LoadSkyBox(bool isHDR, const std::filesystem::path& right, const std::filesystem::path& left, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& front, const std::filesystem::path& back)
//{
//	name = "Skybox";
//	stbi_set_flip_vertically_on_load(false);
//
//	//unsigned char* data[6];
//	std::array<void*, 6> data;
//	int texChannels;
//	if (isHDR)
//	{
//		data[0] = stbi_loadf(right.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[1] = stbi_loadf(left.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[2] = stbi_loadf(top.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[3] = stbi_loadf(bottom.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[4] = stbi_loadf(back.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[5] = stbi_loadf(front.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//	}
//	else
//	{
//		data[0] = stbi_load(right.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[1] = stbi_load(left.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[2] = stbi_load(top.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[3] = stbi_load(bottom.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[4] = stbi_load(back.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//		data[5] = stbi_load(front.string().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
//	}
//
//	//FlipTextureHorizontally(data[0], width, height, STBI_rgb_alpha);
//	//FlipTextureHorizontally(data[1], width, height, STBI_rgb_alpha);
//	//FlipTextureHorizontally(data[4], width, height, STBI_rgb_alpha);
//	//FlipTextureHorizontally(data[5], width, height, STBI_rgb_alpha);
//
//	{
//		//Define an image to create
//		VkImageCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//		createInfo.imageType = VK_IMAGE_TYPE_2D;
//		if (isHDR) createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//		else createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//		createInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
//		createInfo.mipLevels = 1;
//		createInfo.arrayLayers = 6; //6 Layers for CubeMap
//		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//		//Use Optimal Tiling to make GPU effectively process image
//		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//		//Usage for copying and shader
//		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
//
//		//Create image
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImage);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Image Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Declare a variable which will take memory requirements
//		VkMemoryRequirements requirements{};
//		//Get Memory Requirements for Image
//		vkGetImageMemoryRequirements(*vkInit->GetDevice(), vkTextureImage, &requirements);
//
//		//Create Memory Allocation Info
//		VkMemoryAllocateInfo allocateInfo{};
//		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		allocateInfo.allocationSize = requirements.size;
//		//Select memory type which has fast access from GPU
//		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//
//		//Allocate Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkTextureDeviceMemory);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_TOO_MANY_OBJECTS:
//					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Texture Memory Allocation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Bind Image and Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkBindImageMemory(*vkInit->GetDevice(), vkTextureImage, vkTextureDeviceMemory, 0);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Memory Bind Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//	}
//
//	//--------------------Staging Buffer--------------------//
//
//	VkBuffer vkStagingBuffer;
//	VkDeviceSize imageSize{ 0 };
//	if (isHDR) imageSize = width * height * STBI_rgb_alpha * sizeof(float);
//	else imageSize = width * height * STBI_rgb_alpha;
//	VkDeviceSize totalSize = imageSize * 6;
//	VkDeviceMemory vkStagingDeviceMemory;
//	{
//		//Create Staging Buffer Info
//		VkBufferCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//		createInfo.size = totalSize;
//		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
//		//Create Staging Buffer
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateBuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkStagingBuffer);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Staging Buffer Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Declare a variable which will take memory requirements
//		VkMemoryRequirements requirements;
//		//Get Memory Requirements for Buffer
//		vkGetBufferMemoryRequirements(*vkInit->GetDevice(), vkStagingBuffer, &requirements);
//
//		//Create Memory Allocation Info
//		VkMemoryAllocateInfo allocateInfo{};
//		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		allocateInfo.allocationSize = requirements.size;
//		//Select memory type which CPU can access and ensures memory sync between CPU and GPU
//		allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//		//Allocate Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkStagingDeviceMemory);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_TOO_MANY_OBJECTS:
//					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Staging Memory Allocation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Bind Buffer and Memory
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkBindBufferMemory(*vkInit->GetDevice(), vkStagingBuffer, vkStagingDeviceMemory, 0);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Memory Bind Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Get Virtual Address for CPU to access Memory
//		void* contents;
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkMapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, 0, totalSize, 0, &contents);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_MEMORY_MAP_FAILED:
//					std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Memory Map Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//
//		//Copy Bitmap Info to Memory
//		for (int i = 0; i < 6; ++i)
//		{
//			memcpy(static_cast<unsigned char*>(contents) + i * imageSize, data[i], imageSize);
//		}
//
//		//End Accessing Memory from CPU
//		vkUnmapMemory(*vkInit->GetDevice(), vkStagingDeviceMemory);
//	}
//
//	//Deallocate memory for reading image
//	for (int i = 0; i < 6; ++i)
//		stbi_image_free(data[i]);
//
//	//Image is in staging buffer so that processing which copies staging buffer's content to image is needed
//	{
//		//Create CommandBuffer Allocate Info
//		VkCommandBufferAllocateInfo allocateInfo{};
//		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//		allocateInfo.commandPool = *vkCommandPool;
//		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//		allocateInfo.commandBufferCount = 1;
//
//		//Create CommandBuffer
//		VkCommandBuffer commandBuffer;
//		vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &commandBuffer);
//
//		//Create CommandBuffer Begin Info
//		VkCommandBufferBeginInfo beginInfo{};
//		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//		//Begin CommandBuffer
//		vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//		{
//			//Create Image Memory Barrier
//			//Layout should be TRANSFER_DST_OPTIMAL to do copy
//			VkImageMemoryBarrier barrier{};
//			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//			barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.image = vkTextureImage;
//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			barrier.subresourceRange.levelCount = 1;
//			barrier.subresourceRange.baseArrayLayer = 0;
//			barrier.subresourceRange.layerCount = 6;
//
//			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//		}
//
//		//Define Copy Region
//		std::vector<VkBufferImageCopy> regions{};
//		for (int i = 0; i < 6; ++i)
//		{
//			//Define Image Subresource which will be copied
//			VkImageSubresourceLayers subresourceLayers{};
//			subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			subresourceLayers.mipLevel = 0;
//			subresourceLayers.layerCount = 1;
//			subresourceLayers.baseArrayLayer = i;
//
//			//It assumes region has same size as image if buffer region is not defined
//			VkBufferImageCopy region{};
//			region.imageSubresource = subresourceLayers;
//			region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
//			region.imageOffset = { 0, 0, 0 };
//			region.bufferOffset = i * imageSize;
//			regions.push_back(region);
//		}
//
//		//Copy Staging Buffer to Image by defined region
//		vkCmdCopyBufferToImage(commandBuffer, vkStagingBuffer, vkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data());
//
//		{
//			//Create Image Memory Barrier
//			//Layout should be SHADER_READ_ONLY_OPTIMAL to be used at Shader
//			VkImageMemoryBarrier barrier{};
//			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//			barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//			barrier.image = vkTextureImage;
//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			barrier.subresourceRange.levelCount = 1;
//			barrier.subresourceRange.layerCount = 6;
//
//			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//		}
//
//		//End CommandBuffer
//		vkEndCommandBuffer(commandBuffer);
//
//		//Create submit info
//		VkSubmitInfo submitInfo{};
//		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//		submitInfo.commandBufferCount = 1;
//		submitInfo.pCommandBuffers = &commandBuffer;
//
//		//Submit queue to command buffer
//		vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);
//
//		//Wait until all submitted command buffers are handled
//		vkDeviceWaitIdle(*vkInit->GetDevice());
//
//		//Free CommandBuffer
//		vkFreeCommandBuffers(*vkInit->GetDevice(), *vkCommandPool, 1, &commandBuffer);
//
//		//Free Memory
//		vkFreeMemory(*vkInit->GetDevice(), vkStagingDeviceMemory, nullptr);
//
//		//Destroy Staging Buffer
//		vkDestroyBuffer(*vkInit->GetDevice(), vkStagingBuffer, nullptr);
//	}
//
//	{
//		//To access image from graphics pipeline, Image View is needed
//		//Create ImageView Info
//		VkImageViewCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		createInfo.image = vkTextureImage;
//		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
//		if (isHDR) createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//		else createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		createInfo.subresourceRange.levelCount = 1;
//		createInfo.subresourceRange.layerCount = 6;
//		createInfo.subresourceRange.baseArrayLayer = 0;
//
//		//Create ImageView
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageView);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Image View Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//	}
//
//	{
//		//Sampler is needed for shader to read image
//		//Create Sampler Info
//		VkSamplerCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//		if (isHDR)
//		{
//			createInfo.magFilter = VK_FILTER_LINEAR;
//			createInfo.minFilter = VK_FILTER_LINEAR;
//		}
//		else
//		{
//			createInfo.magFilter = VK_FILTER_NEAREST;
//			createInfo.minFilter = VK_FILTER_NEAREST;
//		}
//		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//		createInfo.unnormalizedCoordinates = VK_FALSE;
//
//		//Create Sampler
//		try
//		{
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureSampler);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_TOO_MANY_OBJECTS:
//					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Image Sampler Creation Failed" };
//			}
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << e.what() << std::endl;
//			VKTexture::~VKTexture();
//			std::exit(EXIT_FAILURE);
//		}
//	}
//}
