//Author: JEYOON YU
//Project: CubeEngine
//File: VKSkybox.cpp
#include "VKTexture.hpp"
#include "VKSkybox.hpp"
#include "VKInit.hpp"
#include "VKDescriptor.hpp"
#include "VKPipeLine.hpp"
#include "VKShader.hpp"
#include "VKVertexBuffer.hpp"
#include "VKUniformBuffer.hpp"
#include <iostream>
#include <filesystem>

VKSkybox::VKSkybox(const std::filesystem::path& path, VKInit* init_, VkCommandPool* pool_) : vkInit(init_), vkCommandPool(pool_)
{
	skyboxTexture = new VKTexture(vkInit, vkCommandPool);
	skyboxTexture->LoadTexture(true, path, "skybox", true);
	faceSize = skyboxTexture->GetHeight();

	//Create command buffer info
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = *vkCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	//Create command buffer
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &skyboxCommandBuffer);
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

			throw std::runtime_error{ "Command Buffer Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//projection[1][1] *= -1.0f;
	EquirectangularToCube(&skyboxCommandBuffer);
	CalculateIrradiance(&skyboxCommandBuffer);
	PrefilteredEnvironmentMap(&skyboxCommandBuffer);
	BRDFLUT(&skyboxCommandBuffer);
}

VKSkybox::~VKSkybox()
{
	//Destroy Sampler
	vkDestroySampler(*vkInit->GetDevice(), vkTextureSamplerEquirectangular, nullptr);
	vkDestroySampler(*vkInit->GetDevice(), vkTextureSamplerIrradiance, nullptr);
	vkDestroySampler(*vkInit->GetDevice(), vkTextureSamplerPrefilter, nullptr);
	vkDestroySampler(*vkInit->GetDevice(), vkTextureSamplerBRDFLUT, nullptr);
	//Destroy ImageView
	vkDestroyImageView(*vkInit->GetDevice(), vkTextureImageViewEquirectangular, nullptr);
	vkDestroyImageView(*vkInit->GetDevice(), vkTextureImageViewIrradiance, nullptr);
	vkDestroyImageView(*vkInit->GetDevice(), vkTextureImageViewPrefilter, nullptr);
	vkDestroyImageView(*vkInit->GetDevice(), vkTextureImageViewBRDFLUT, nullptr);
	//Free Memory
	vkFreeMemory(*vkInit->GetDevice(), vkTextureDeviceMemoryEquirectangular, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), vkTextureDeviceMemoryIrradiance, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), vkTextureDeviceMemoryPrefilter, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), vkTextureDeviceMemoryBRDFLUT, nullptr);
	//Destroy Image
	vkDestroyImage(*vkInit->GetDevice(), vkTextureImageEquirectangular, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), vkTextureImageIrradiance, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), vkTextureImagePrefilter, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), vkTextureImageBRDFLUT, nullptr);

	//for (auto& view : cubeFaceViews)
	//{
	//	vkDestroyImageView(*vkInit->GetDevice(), view, nullptr);
	//}

	//vkDestroyRenderPass(*vkInit->GetDevice(), renderPassIBL, nullptr);

	//for (auto& fb : cubeFaceFramebuffers)
	//{
	//	vkDestroyFramebuffer(*vkInit->GetDevice(), fb, nullptr);
	//}

	delete skyboxTexture;

	vkFreeCommandBuffers(*vkInit->GetDevice(), *vkCommandPool, 1, &skyboxCommandBuffer);
}

uint32_t VKSkybox::FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_)
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

void VKSkybox::EquirectangularToCube(VkCommandBuffer* commandBuffer)
{
	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		createInfo.extent = { faceSize, faceSize, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 6; //6 Layers for CubeMap
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying, shader, and renderpass
		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		//Create image
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageEquirectangular);
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

				throw std::runtime_error{ "Image Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Declare a variable which will take memory requirements
	VkMemoryRequirements requirements{};
	//Get Memory Requirements for Image
	vkGetImageMemoryRequirements(*vkInit->GetDevice(), vkTextureImageEquirectangular, &requirements);

	//Create Memory Allocation Info
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	//Select memory type which has fast access from GPU
	allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//Allocate Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkTextureDeviceMemoryEquirectangular);
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

			throw std::runtime_error{ "Texture Memory Allocation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//Bind Image and Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkBindImageMemory(*vkInit->GetDevice(), vkTextureImageEquirectangular, vkTextureDeviceMemoryEquirectangular, 0);
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
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	{
		//To access image from graphics pipeline, Image View is needed
		//Create ImageView Info
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vkTextureImageEquirectangular;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.layerCount = 6;
		createInfo.subresourceRange.baseArrayLayer = 0;

		//Create ImageView
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageViewEquirectangular);
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

				throw std::runtime_error{ "Image View Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	{
		//Sampler is needed for shader to read image
		//Create Sampler Info
		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		//createInfo.anisotropyEnable = VK_TRUE;
		//createInfo.maxAnisotropy = 16.f;
		//createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		//Create Sampler
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureSamplerEquirectangular);
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

				throw std::runtime_error{ "Image Sampler Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Prepare RenderPass
	//Create Attachment Description
	VkAttachmentDescription colorAattachmentDescription{};
	colorAattachmentDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorAattachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAattachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAattachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAattachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAattachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;

	VkRenderPassCreateInfo rpCreateInfo{};
	rpCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpCreateInfo.attachmentCount = 1;
	rpCreateInfo.pAttachments = &colorAattachmentDescription;
	rpCreateInfo.subpassCount = 1;
	rpCreateInfo.pSubpasses = &subpassDescription;

	//Create Renderpass
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateRenderPass(*vkInit->GetDevice(), &rpCreateInfo, nullptr, &renderPassIBL);
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

			throw std::runtime_error{ "RenderPass Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	for (uint32_t f = 0; f < 6; ++f)
	{
		//Create ImageView
		try
		{
			//To access image from graphics pipeline, Image View is needed
			//Create ImageView Info
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = vkTextureImageEquirectangular;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.subresourceRange.baseArrayLayer = f;

			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &cubeFaceViews[f]);
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

				throw std::runtime_error{ "Image View Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	for (uint32_t f = 0; f < 6; ++f)
	{
		VkImageView attachments[] = { cubeFaceViews[f] };

		//Create framebuffer info
		VkFramebufferCreateInfo fbCreateInfo{};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = renderPassIBL;
		fbCreateInfo.attachmentCount = 1;
		fbCreateInfo.pAttachments = attachments;
		fbCreateInfo.width = faceSize;
		fbCreateInfo.height = faceSize;
		fbCreateInfo.layers = 1;

		try
		{
			//Create framebuffer
			VkResult result{ VK_SUCCESS };
			result = vkCreateFramebuffer(*vkInit->GetDevice(), &fbCreateInfo, nullptr, &cubeFaceFramebuffers[f]);
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

				throw std::runtime_error{ "Framebuffer Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Render Images to Cube
	VKShader shaderIBL{ vkInit->GetDevice() };
	shaderIBL.LoadShader("../Engine/shaders/glsl/Cubemap.vert", "../Engine/shaders/glsl/Equirectangular.frag");

	VKDescriptorLayout fragmentLayout;
	fragmentLayout.descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout.descriptorCount = 1;
	VKDescriptor descriptorIBL{ vkInit, {}, { fragmentLayout } };

	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = 0;

	VKPipeLine pipelineIBL{ vkInit->GetDevice(), descriptorIBL.GetDescriptorSetLayout() };
	VkExtent2D extentIBL{ faceSize, faceSize };
	pipelineIBL.InitPipeLine(shaderIBL.GetVertexModule(), shaderIBL.GetFragmentModule(), &extentIBL, &renderPassIBL, sizeof(float) * 3, { position_layout }, VK_SAMPLE_COUNT_1_BIT, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, true, sizeof(glm::mat4) * 2, VK_SHADER_STAGE_VERTEX_BIT);

	VKVertexBuffer<glm::vec3> vertexBufferIBL{ vkInit, &skyboxVertices };

	VkWriteDescriptorSet descriptorWrite{};
	VkDescriptorImageInfo skyboxDescriptorImageInfo{};
	skyboxDescriptorImageInfo.sampler = *skyboxTexture->GetSampler();
	skyboxDescriptorImageInfo.imageView = *skyboxTexture->GetImageView();
	skyboxDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which resource descriptor set will point
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = (*descriptorIBL.GetFragmentDescriptorSets())[0];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.pImageInfo = &skyboxDescriptorImageInfo;

	//Update DescriptorSet
	//DescriptorSet does not have to update every frame since it points same uniform buffer
	vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);

	//Create Viewport and Scissor for Dynamic State
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(faceSize);
	viewport.height = static_cast<float>(faceSize);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { faceSize, faceSize };

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	vkBeginCommandBuffer(*commandBuffer, &beginInfo);

	//Create renderpass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = renderPassIBL;
	renderpassBeginInfo.renderArea.offset = { 0, 0 };
	renderpassBeginInfo.renderArea.extent = { faceSize, faceSize };
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearColor;

	for (int f = 0; f < 6; ++f)
	{
		renderpassBeginInfo.framebuffer = cubeFaceFramebuffers[f];

		vkCmdBeginRenderPass(*commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//Bind Vertex Buffer
		VkDeviceSize vertexBufferOffset{ 0 };
		vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBufferIBL.GetVertexBuffer(), &vertexBufferOffset);
		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLine());
		//Dynamic Viewport & Scissor
		vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLineLayout(), 0, 1, &(*descriptorIBL.GetFragmentDescriptorSets())[0], 0, nullptr);
		//Push Constant World-To_NDC
		glm::mat4 transform[2] = { views[f], projection};
		vkCmdPushConstants(*commandBuffer, *pipelineIBL.GetPipeLineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) * 2, &transform[0]);

		vkCmdDraw(*commandBuffer, 36, 1, 0, 0);

		vkCmdEndRenderPass(*commandBuffer);
	}

	vkEndCommandBuffer(*commandBuffer);

	//Create submit info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffer;

	//Submit queue to command buffer
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//Deallocate Resources
	for (auto& view : cubeFaceViews)
	{
		vkDestroyImageView(*vkInit->GetDevice(), view, nullptr);
	}

	vkDestroyRenderPass(*vkInit->GetDevice(), renderPassIBL, nullptr);

	for (auto& fb : cubeFaceFramebuffers)
	{
		vkDestroyFramebuffer(*vkInit->GetDevice(), fb, nullptr);
	}
}

void VKSkybox::CalculateIrradiance(VkCommandBuffer* commandBuffer)
{
	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		createInfo.extent = { irradianceSize, irradianceSize, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 6; //6 Layers for CubeMap
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying, shader, and renderpass
		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		//Create image
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageIrradiance);
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

				throw std::runtime_error{ "Image Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Declare a variable which will take memory requirements
	VkMemoryRequirements requirements{};
	//Get Memory Requirements for Image
	vkGetImageMemoryRequirements(*vkInit->GetDevice(), vkTextureImageIrradiance, &requirements);

	//Create Memory Allocation Info
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	//Select memory type which has fast access from GPU
	allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//Allocate Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkTextureDeviceMemoryIrradiance);
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

			throw std::runtime_error{ "Texture Memory Allocation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//Bind Image and Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkBindImageMemory(*vkInit->GetDevice(), vkTextureImageIrradiance, vkTextureDeviceMemoryIrradiance, 0);
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
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	{
		//To access image from graphics pipeline, Image View is needed
		//Create ImageView Info
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vkTextureImageIrradiance;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.layerCount = 6;
		createInfo.subresourceRange.baseArrayLayer = 0;

		//Create ImageView
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageViewIrradiance);
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

				throw std::runtime_error{ "Image View Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	{
		//Sampler is needed for shader to read image
		//Create Sampler Info
		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		//createInfo.anisotropyEnable = VK_TRUE;
		//createInfo.maxAnisotropy = 16.f;
		//createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		//Create Sampler
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureSamplerIrradiance);
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

				throw std::runtime_error{ "Image Sampler Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Prepare RenderPass
	//Create Attachment Description
	VkAttachmentDescription colorAattachmentDescription{};
	colorAattachmentDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorAattachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAattachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAattachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAattachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAattachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;

	VkRenderPassCreateInfo rpCreateInfo{};
	rpCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpCreateInfo.attachmentCount = 1;
	rpCreateInfo.pAttachments = &colorAattachmentDescription;
	rpCreateInfo.subpassCount = 1;
	rpCreateInfo.pSubpasses = &subpassDescription;

	//Create Renderpass
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateRenderPass(*vkInit->GetDevice(), &rpCreateInfo, nullptr, &renderPassIBL);
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

			throw std::runtime_error{ "RenderPass Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	for (uint32_t f = 0; f < 6; ++f)
	{
		//Create ImageView
		try
		{
			//To access image from graphics pipeline, Image View is needed
			//Create ImageView Info
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = vkTextureImageIrradiance;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.subresourceRange.baseArrayLayer = f;

			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &cubeFaceViews[f]);
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

				throw std::runtime_error{ "Image View Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	for (uint32_t f = 0; f < 6; ++f)
	{
		VkImageView attachments[] = { cubeFaceViews[f] };

		//Create framebuffer info
		VkFramebufferCreateInfo fbCreateInfo{};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = renderPassIBL;
		fbCreateInfo.attachmentCount = 1;
		fbCreateInfo.pAttachments = attachments;
		fbCreateInfo.width = irradianceSize;
		fbCreateInfo.height = irradianceSize;
		fbCreateInfo.layers = 1;

		try
		{
			//Create framebuffer
			VkResult result{ VK_SUCCESS };
			result = vkCreateFramebuffer(*vkInit->GetDevice(), &fbCreateInfo, nullptr, &cubeFaceFramebuffers[f]);
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

				throw std::runtime_error{ "Framebuffer Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Render Images to Cube
	VKShader shaderIBL{ vkInit->GetDevice() };
	shaderIBL.LoadShader("../Engine/shaders/glsl/Cubemap.vert", "../Engine/shaders/glsl/Irradiance.frag");

	VKDescriptorLayout fragmentLayout;
	fragmentLayout.descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout.descriptorCount = 1;
	VKDescriptor descriptorIBL{ vkInit, {}, { fragmentLayout } };

	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = 0;

	VKPipeLine pipelineIBL{ vkInit->GetDevice(), descriptorIBL.GetDescriptorSetLayout() };
	VkExtent2D extentIBL{ irradianceSize, irradianceSize };
	pipelineIBL.InitPipeLine(shaderIBL.GetVertexModule(), shaderIBL.GetFragmentModule(), &extentIBL, &renderPassIBL, sizeof(float) * 3, { position_layout }, VK_SAMPLE_COUNT_1_BIT, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, true, sizeof(glm::mat4) * 2, VK_SHADER_STAGE_VERTEX_BIT);

	VKVertexBuffer<glm::vec3> vertexBufferIBL{ vkInit, &skyboxVertices };

	VkWriteDescriptorSet descriptorWrite{};
	VkDescriptorImageInfo skyboxDescriptorImageInfo{};
	skyboxDescriptorImageInfo.sampler = *GetCubeMap().first;
	skyboxDescriptorImageInfo.imageView = *GetCubeMap().second;
	skyboxDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which resource descriptor set will point
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = (*descriptorIBL.GetFragmentDescriptorSets())[0];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.pImageInfo = &skyboxDescriptorImageInfo;

	//Update DescriptorSet
	//DescriptorSet does not have to update every frame since it points same uniform buffer
	vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);

	//Create Viewport and Scissor for Dynamic State
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(irradianceSize);
	viewport.height = static_cast<float>(irradianceSize);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { irradianceSize, irradianceSize };

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	vkBeginCommandBuffer(*commandBuffer, &beginInfo);

	//Create renderpass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = renderPassIBL;
	renderpassBeginInfo.renderArea.offset = { 0, 0 };
	renderpassBeginInfo.renderArea.extent = { irradianceSize, irradianceSize };
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearColor;

	for (int f = 0; f < 6; ++f)
	{
		renderpassBeginInfo.framebuffer = cubeFaceFramebuffers[f];

		vkCmdBeginRenderPass(*commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//Bind Vertex Buffer
		VkDeviceSize vertexBufferOffset{ 0 };
		vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBufferIBL.GetVertexBuffer(), &vertexBufferOffset);
		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLine());
		//Dynamic Viewport & Scissor
		vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLineLayout(), 0, 1, &(*descriptorIBL.GetFragmentDescriptorSets())[0], 0, nullptr);
		//Push Constant World-To_NDC
		glm::mat4 transform[2] = { views[f], projection };
		vkCmdPushConstants(*commandBuffer, *pipelineIBL.GetPipeLineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) * 2, &transform[0]);

		vkCmdDraw(*commandBuffer, 36, 1, 0, 0);

		vkCmdEndRenderPass(*commandBuffer);
	}

	vkEndCommandBuffer(*commandBuffer);

	//Create submit info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffer;

	//Submit queue to command buffer
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//Deallocate Resources
	for (auto& view : cubeFaceViews)
	{
		vkDestroyImageView(*vkInit->GetDevice(), view, nullptr);
	}

	vkDestroyRenderPass(*vkInit->GetDevice(), renderPassIBL, nullptr);

	for (auto& fb : cubeFaceFramebuffers)
	{
		vkDestroyFramebuffer(*vkInit->GetDevice(), fb, nullptr);
	}
}

void VKSkybox::PrefilteredEnvironmentMap(VkCommandBuffer* commandBuffer)
{
	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		createInfo.extent = { baseSize, baseSize, 1 };
		createInfo.mipLevels = mipLevels;
		createInfo.arrayLayers = 6; //6 Layers for CubeMap
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying, shader, and renderpass
		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		//Create image
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImagePrefilter);
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

				throw std::runtime_error{ "Image Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Declare a variable which will take memory requirements
	VkMemoryRequirements requirements{};
	//Get Memory Requirements for Image
	vkGetImageMemoryRequirements(*vkInit->GetDevice(), vkTextureImagePrefilter, &requirements);

	//Create Memory Allocation Info
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	//Select memory type which has fast access from GPU
	allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//Allocate Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkTextureDeviceMemoryPrefilter);
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

			throw std::runtime_error{ "Texture Memory Allocation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//Bind Image and Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkBindImageMemory(*vkInit->GetDevice(), vkTextureImagePrefilter, vkTextureDeviceMemoryPrefilter, 0);
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
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	{
		//To access image from graphics pipeline, Image View is needed
		//Create ImageView Info
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vkTextureImagePrefilter;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.layerCount = 6;
		createInfo.subresourceRange.baseArrayLayer = 0;

		//Create ImageView
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageViewPrefilter);
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

				throw std::runtime_error{ "Image View Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	{
		//Sampler is needed for shader to read image
		//Create Sampler Info
		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.minLod = 0.f;
		createInfo.maxLod = static_cast<float>(mipLevels);
		createInfo.mipLodBias = 0.f;
		//createInfo.anisotropyEnable = VK_TRUE;
		//createInfo.maxAnisotropy = 16.f;
		//createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		//Create Sampler
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureSamplerPrefilter);
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

				throw std::runtime_error{ "Image Sampler Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Prepare RenderPass
	//Create Attachment Description
	VkAttachmentDescription colorAattachmentDescription{};
	colorAattachmentDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorAattachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAattachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAattachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAattachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAattachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;

	VkRenderPassCreateInfo rpCreateInfo{};
	rpCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpCreateInfo.attachmentCount = 1;
	rpCreateInfo.pAttachments = &colorAattachmentDescription;
	rpCreateInfo.subpassCount = 1;
	rpCreateInfo.pSubpasses = &subpassDescription;

	//Create Renderpass
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateRenderPass(*vkInit->GetDevice(), &rpCreateInfo, nullptr, &renderPassIBL);
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

			throw std::runtime_error{ "RenderPass Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//Render Images to Cube
	VKShader shaderIBL{ vkInit->GetDevice() };
	shaderIBL.LoadShader("../Engine/shaders/glsl/Cubemap.vert", "../Engine/shaders/glsl/Prefilter.frag");

	VKDescriptorLayout fragmentLayout[2];
	fragmentLayout[0].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[0].descriptorCount = 1;
	fragmentLayout[1].descriptorType = VKDescriptorLayout::UNIFORM;
	fragmentLayout[1].descriptorCount = 1;
	VKDescriptor descriptorIBL{ vkInit, {}, { fragmentLayout[0], fragmentLayout[1] }};

	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = 0;

	VKPipeLine pipelineIBL{ vkInit->GetDevice(), descriptorIBL.GetDescriptorSetLayout() };
	VkExtent2D extentIBL{ baseSize, baseSize };
	pipelineIBL.InitPipeLine(shaderIBL.GetVertexModule(), shaderIBL.GetFragmentModule(), &extentIBL, &renderPassIBL, sizeof(float) * 3, { position_layout }, VK_SAMPLE_COUNT_1_BIT, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, true, sizeof(glm::mat4) * 2, VK_SHADER_STAGE_VERTEX_BIT);

	VKVertexBuffer<glm::vec3> vertexBufferIBL{ vkInit, &skyboxVertices };

	VKUniformBuffer<float> prefilterUniform{ vkInit, 1 };

	VkWriteDescriptorSet descriptorWrite[2]{};
	VkDescriptorImageInfo skyboxDescriptorImageInfo{};
	skyboxDescriptorImageInfo.sampler = *GetCubeMap().first;
	skyboxDescriptorImageInfo.imageView = *GetCubeMap().second;
	skyboxDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which resource descriptor set will point
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].dstSet = (*descriptorIBL.GetFragmentDescriptorSets())[0];
	descriptorWrite[0].dstBinding = 0;
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[0].pImageInfo = &skyboxDescriptorImageInfo;

	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = (*(prefilterUniform.GetUniformBuffers()))[0];
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(float);

	//Define which resource descriptor set will point
	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].dstSet = (*descriptorIBL.GetFragmentDescriptorSets())[0];
	descriptorWrite[1].dstBinding = 1;
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[1].pBufferInfo = &bufferInfo;

	//Update DescriptorSet
	//DescriptorSet does not have to update every frame since it points same uniform buffer
	vkUpdateDescriptorSets(*vkInit->GetDevice(), 2, &descriptorWrite[0], 0, nullptr);

	for (uint32_t mip = 0; mip < mipLevels; ++mip)
	{
		uint32_t dim = baseSize >> mip;
		float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);
		prefilterUniform.UpdateUniform(1, &roughness, 0);

		//Create command buffer begin info
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		//Begin command buffer
		vkBeginCommandBuffer(*commandBuffer, &beginInfo);

		//Create renderpass begin info
		VkRenderPassBeginInfo renderpassBeginInfo{};
		renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassBeginInfo.renderPass = renderPassIBL;
		renderpassBeginInfo.renderArea.offset = { 0, 0 };
		renderpassBeginInfo.renderArea.extent = { dim, dim };
		renderpassBeginInfo.clearValueCount = 1;
		renderpassBeginInfo.pClearValues = &clearColor;

		for (uint32_t f = 0; f < 6; ++f)
		{
			//Create ImageView
			try
			{
				//To access image from graphics pipeline, Image View is needed
				//Create ImageView Info
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = vkTextureImagePrefilter;
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.layerCount = 1;
				createInfo.subresourceRange.baseArrayLayer = f;
				createInfo.subresourceRange.baseMipLevel = mip;

				VkResult result{ VK_SUCCESS };
				result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &cubeFaceViews[f]);
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

					throw std::runtime_error{ "Image View Creation Failed" };
				}
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				VKSkybox::~VKSkybox();
				std::exit(EXIT_FAILURE);
			}

			//Create framebuffer info
			VkImageView attachments[] = { cubeFaceViews[f] };
			VkFramebufferCreateInfo fbCreateInfo{};
			fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbCreateInfo.renderPass = renderPassIBL;
			fbCreateInfo.attachmentCount = 1;
			fbCreateInfo.pAttachments = attachments;
			fbCreateInfo.width = dim;
			fbCreateInfo.height = dim;
			fbCreateInfo.layers = 1;

			try
			{
				//Create framebuffer
				VkResult result{ VK_SUCCESS };
				result = vkCreateFramebuffer(*vkInit->GetDevice(), &fbCreateInfo, nullptr, &cubeFaceFramebuffers[f]);
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

					throw std::runtime_error{ "Framebuffer Creation Failed" };
				}
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				VKSkybox::~VKSkybox();
				std::exit(EXIT_FAILURE);
			}

			//Create Viewport and Scissor for Dynamic State
			VkViewport viewport{};
			viewport.x = 0.f;
			viewport.y = 0.f;
			viewport.width = static_cast<float>(dim);
			viewport.height = static_cast<float>(dim);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = { dim, dim };

			renderpassBeginInfo.framebuffer = cubeFaceFramebuffers[f];

			vkCmdBeginRenderPass(*commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			//Bind Vertex Buffer
			VkDeviceSize vertexBufferOffset{ 0 };
			vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBufferIBL.GetVertexBuffer(), &vertexBufferOffset);
			vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLine());
			//Dynamic Viewport & Scissor
			vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
			vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLineLayout(), 0, 1, &(*descriptorIBL.GetFragmentDescriptorSets())[0], 0, nullptr);
			//Push Constant World-To_NDC
			glm::mat4 transform[2] = { views[f], projection };
			vkCmdPushConstants(*commandBuffer, *pipelineIBL.GetPipeLineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) * 2, &transform[0]);

			vkCmdDraw(*commandBuffer, 36, 1, 0, 0);

			vkCmdEndRenderPass(*commandBuffer);
		}

		vkEndCommandBuffer(*commandBuffer);

		//Create submit info
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer;

		//Submit queue to command buffer
		vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		//Wait until all submitted command buffers are handled
		vkDeviceWaitIdle(*vkInit->GetDevice());

		//Deallocate Resources
		for (auto& view : cubeFaceViews)
		{
			vkDestroyImageView(*vkInit->GetDevice(), view, nullptr);
		}

		for (auto& fb : cubeFaceFramebuffers)
		{
			vkDestroyFramebuffer(*vkInit->GetDevice(), fb, nullptr);
		}
	}

	vkDestroyRenderPass(*vkInit->GetDevice(), renderPassIBL, nullptr);
}

void VKSkybox::BRDFLUT(VkCommandBuffer* commandBuffer)
{
	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = VK_FORMAT_R16G16_SFLOAT;
		createInfo.extent = { lutSize, lutSize, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying, shader, and renderpass
		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		//Create image
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageBRDFLUT);
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

				throw std::runtime_error{ "Image Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Declare a variable which will take memory requirements
	VkMemoryRequirements requirements{};
	//Get Memory Requirements for Image
	vkGetImageMemoryRequirements(*vkInit->GetDevice(), vkTextureImageBRDFLUT, &requirements);

	//Create Memory Allocation Info
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	//Select memory type which has fast access from GPU
	allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//Allocate Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &vkTextureDeviceMemoryBRDFLUT);
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

			throw std::runtime_error{ "Texture Memory Allocation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//Bind Image and Memory
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkBindImageMemory(*vkInit->GetDevice(), vkTextureImageBRDFLUT, vkTextureDeviceMemoryBRDFLUT, 0);
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
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	{
		//To access image from graphics pipeline, Image View is needed
		//Create ImageView Info
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vkTextureImageBRDFLUT;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = VK_FORMAT_R16G16_SFLOAT;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;

		//Create ImageView
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureImageViewBRDFLUT);
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

				throw std::runtime_error{ "Image View Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	{
		//Sampler is needed for shader to read image
		//Create Sampler Info
		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		//createInfo.anisotropyEnable = VK_TRUE;
		//createInfo.maxAnisotropy = 16.f;
		//createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		//Create Sampler
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureSamplerBRDFLUT);
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

				throw std::runtime_error{ "Image Sampler Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKSkybox::~VKSkybox();
			std::exit(EXIT_FAILURE);
		}
	}

	//Prepare RenderPass
	//Create Attachment Description
	VkAttachmentDescription colorAattachmentDescription{};
	colorAattachmentDescription.format = VK_FORMAT_R16G16_SFLOAT;
	colorAattachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAattachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAattachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAattachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAattachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;

	VkRenderPassCreateInfo rpCreateInfo{};
	rpCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpCreateInfo.attachmentCount = 1;
	rpCreateInfo.pAttachments = &colorAattachmentDescription;
	rpCreateInfo.subpassCount = 1;
	rpCreateInfo.pSubpasses = &subpassDescription;

	//Create Renderpass
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateRenderPass(*vkInit->GetDevice(), &rpCreateInfo, nullptr, &renderPassIBL);
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

			throw std::runtime_error{ "RenderPass Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	VkImageView attachments[] = { vkTextureImageViewBRDFLUT };

	//Create framebuffer info
	VkFramebufferCreateInfo fbCreateInfo{};
	fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCreateInfo.renderPass = renderPassIBL;
	fbCreateInfo.attachmentCount = 1;
	fbCreateInfo.pAttachments = attachments;
	fbCreateInfo.width = lutSize;
	fbCreateInfo.height = lutSize;
	fbCreateInfo.layers = 1;

	VkFramebuffer fb;

	try
	{
		//Create framebuffer
		VkResult result{ VK_SUCCESS };
		result = vkCreateFramebuffer(*vkInit->GetDevice(), &fbCreateInfo, nullptr, &fb);
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

			throw std::runtime_error{ "Framebuffer Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSkybox::~VKSkybox();
		std::exit(EXIT_FAILURE);
	}

	//Render Images to Cube
	VKShader shaderIBL{ vkInit->GetDevice() };
	shaderIBL.LoadShader("../Engine/shaders/glsl/BRDF.vert", "../Engine/shaders/glsl/BRDF.frag");

	VKDescriptor descriptorIBL{ vkInit, {}, {} };

	struct VA
	{
		glm::vec3 position;
		glm::vec2 texCoord;
	};

	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = offsetof(VA, position);

	VKAttributeLayout texture_layout;
	texture_layout.vertex_layout_location = 1;
	texture_layout.format = VK_FORMAT_R32G32_SFLOAT;
	texture_layout.offset = offsetof(VA, texCoord);

	VKPipeLine pipelineIBL{ vkInit->GetDevice(), descriptorIBL.GetDescriptorSetLayout() };
	VkExtent2D extentIBL{ lutSize, lutSize };
	pipelineIBL.InitPipeLine(shaderIBL.GetVertexModule(), shaderIBL.GetFragmentModule(), &extentIBL, &renderPassIBL, sizeof(VA), { position_layout, texture_layout }, VK_SAMPLE_COUNT_1_BIT, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, false);

	std::vector<VA> vas;
	for (int i = 0; i < 4; ++i)
	{
		vas.push_back({ fullscreenQuad[i], fullscreenQuadTexCoords[i] });
	}

	VKVertexBuffer<VA> vertexBufferIBL{ vkInit, &vas };

	//Create Viewport and Scissor for Dynamic State
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(lutSize);
	viewport.height = static_cast<float>(lutSize);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { lutSize, lutSize };

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	vkBeginCommandBuffer(*commandBuffer, &beginInfo);

	//Create renderpass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = renderPassIBL;
	renderpassBeginInfo.framebuffer = fb;
	renderpassBeginInfo.renderArea.offset = { 0, 0 };
	renderpassBeginInfo.renderArea.extent = { lutSize, lutSize };
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(*commandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//Bind Vertex Buffer
	VkDeviceSize vertexBufferOffset{ 0 };
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBufferIBL.GetVertexBuffer(), &vertexBufferOffset);
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineIBL.GetPipeLine());
	//Dynamic Viewport & Scissor
	vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);

	vkCmdDraw(*commandBuffer, 4, 1, 0, 0);

	vkCmdEndRenderPass(*commandBuffer);

	vkEndCommandBuffer(*commandBuffer);

	//Create submit info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffer;

	//Submit queue to command buffer
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//Deallocate Resources
	vkDestroyRenderPass(*vkInit->GetDevice(), renderPassIBL, nullptr);
	vkDestroyFramebuffer(*vkInit->GetDevice(), fb, nullptr);
}
