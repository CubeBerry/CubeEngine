//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKRenderManager.cpp
#include "VKRenderManager.hpp"
#include "VKInit.hpp"
#include "VKSwapChain.hpp"
#include "VKShader.hpp"
#include "VKPipeLine.hpp"
#include "VKUniformBuffer.hpp"
#include "VKSkybox.hpp"

#include "Engine.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

VKRenderManager::~VKRenderManager()
{
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//delete ImGui
	delete imguiManager;

	//Destroy Command Pool, also Command Buffer destroys with Command Pool
	vkDestroyCommandPool(*vkInit->GetDevice(), vkCommandPool, nullptr);
	//Destroy RenderPass
	vkDestroyRenderPass(*vkInit->GetDevice(), vkRenderPass, nullptr);
	//Destroy FrameBuffer
	for (auto& framebuffer : vkFrameBuffers)
	{
		vkDestroyFramebuffer(*vkInit->GetDevice(), framebuffer, nullptr);
	}

	//Destroy Buffers
	delete pointLightUniformBuffer;
	delete directionalLightUniformBuffer;

	//Destroy Texture
	for (const auto t : textures)
		delete t;

	//Destroy Batch ImageInfo
	vkDestroySampler(*vkInit->GetDevice(), immutableSampler, nullptr);
	//size_t texSize{ textures.size() };
	//for (size_t i = texSize; i < imageInfos.size(); ++i)
	//	vkDestroySampler(*vkInit->GetDevice(), imageInfos[i].sampler, nullptr);

	textures.erase(textures.begin(), textures.end());
	imageInfos.erase(imageInfos.begin(), imageInfos.end());

	//Destroy Depth Buffering
	vkDestroyImageView(*vkInit->GetDevice(), depthImageView, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), depthImageMemory, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), depthImage, nullptr);

	//Destroy MSAA
	vkDestroyImageView(*vkInit->GetDevice(), colorImageView, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), colorImageMemory, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), colorImage, nullptr);

	//Destroy Normal
#ifdef _DEBUG
	delete vkNormal3DShader;
	delete vkPipeline3DNormal;
#endif

	//Destroy Skybox
	if (skyboxEnabled)
	{
		delete skybox;
		delete skyboxShader;
		delete skyboxDescriptor;
		delete vkPipeline3DSkybox;
		delete skyboxVertexBuffer;
	}

	//Destroy Shader
	delete vkShader2D;
	delete vkShader3D;

	//Destroy Pipeline
	delete vkPipeline2D;
	delete vkPipeline3D;
	delete vkPipeline3DLine;

	//Destroy Descriptor
	delete vkDescriptor;

	delete vkSwapChain;
	delete vkInit;
}

void VKRenderManager::CreateDepthBuffer()
{
	depthFormat = FindDepthFormat();

	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = depthFormat;
		createInfo.extent = { vkSwapChain->GetSwapChainImageExtent()->width, vkSwapChain->GetSwapChainImageExtent()->height, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = msaaSamples;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying and shader
		createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//Create image
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &depthImage);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Image Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements{};
		//Get Memory Requirements for Image
		vkGetImageMemoryRequirements(*vkInit->GetDevice(), depthImage, &requirements);

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
			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &depthImageMemory);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Texture Memory Allocation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}

		//Bind Image and Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkBindImageMemory(*vkInit->GetDevice(), depthImage, depthImageMemory, 0);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Memory Bind Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}
	}

	//To access image from graphics pipeline, Image View is needed
	//Create ImageView Info
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = depthImage;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = depthFormat;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	//Create ImageView
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &depthImageView);
		if (result != VK_SUCCESS)
		{
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
				break;
			default:
				break;
			}
			std::cout << '\n';

			throw std::runtime_error{ "Image View Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << '\n';
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}
}

void VKRenderManager::CreateColorResources()
{
	imageFormat = vkInit->SetSurfaceFormat().format;

	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = imageFormat;
		createInfo.extent = { vkSwapChain->GetSwapChainImageExtent()->width, vkSwapChain->GetSwapChainImageExtent()->height, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.samples = msaaSamples;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying and shader
		createInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		//Create image
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateImage(*vkInit->GetDevice(), &createInfo, nullptr, &colorImage);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Image Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements{};
		//Get Memory Requirements for Image
		vkGetImageMemoryRequirements(*vkInit->GetDevice(), colorImage, &requirements);

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
			result = vkAllocateMemory(*vkInit->GetDevice(), &allocateInfo, nullptr, &colorImageMemory);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					std::cout << "VK_ERROR_TOO_MANY_OBJECTS" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Texture Memory Allocation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}

		//Bind Image and Memory
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkBindImageMemory(*vkInit->GetDevice(), colorImage, colorImageMemory, 0);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Memory Bind Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}
	}

	//To access image from graphics pipeline, Image View is needed
	//Create ImageView Info
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = colorImage;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = imageFormat;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	//Create ImageView
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &colorImageView);
		if (result != VK_SUCCESS)
		{
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
				break;
			default:
				break;
			}
			std::cout << '\n';

			throw std::runtime_error{ "Image View Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << '\n';
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}
}

void VKRenderManager::Initialize(SDL_Window* window_)
{
	window = window_;

	vkInit = new VKInit;
	vkInit->Initialize(window);

	InitCommandPool();
	InitCommandBuffer();

	vkSwapChain = new VKSwapChain(vkInit, &vkCommandPool);

	//MSAA
	msaaSamples = GetMaxUsableSampleCount();
	CreateColorResources();
	//Depth Buffering
	CreateDepthBuffer();

	InitRenderPass();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());

	VKDescriptorLayout vertexLayout;
	vertexLayout.descriptorType = VKDescriptorLayout::UNIFORM_DYNAMIC;
	vertexLayout.descriptorCount = 1;

	VKDescriptorLayout fragmentLayout[8];
	fragmentLayout[0].descriptorType = VKDescriptorLayout::UNIFORM_DYNAMIC;
	fragmentLayout[0].descriptorCount = 1;
	fragmentLayout[1].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[1].descriptorCount = 500;
	fragmentLayout[2].descriptorType = VKDescriptorLayout::UNIFORM_DYNAMIC;
	fragmentLayout[2].descriptorCount = 1;
	fragmentLayout[3].descriptorType = VKDescriptorLayout::UNIFORM_DYNAMIC;
	fragmentLayout[3].descriptorCount = 1;
	fragmentLayout[4].descriptorType = VKDescriptorLayout::UNIFORM_DYNAMIC;
	fragmentLayout[4].descriptorCount = 1;
	fragmentLayout[5].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[5].descriptorCount = 1;
	fragmentLayout[6].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[6].descriptorCount = 1;
	fragmentLayout[7].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[7].descriptorCount = 1;
	vkDescriptor = new VKDescriptor(vkInit, { vertexLayout }, { fragmentLayout[0], fragmentLayout[1], fragmentLayout[2], fragmentLayout[3], fragmentLayout[4], fragmentLayout[5], fragmentLayout[6], fragmentLayout[7] });

	vkShader2D = new VKShader(vkInit->GetDevice());
	vkShader2D->LoadShader("../Engine/shaders/spirv/2D.vert.spv", "../Engine/shaders/spirv/2D.frag.spv");

	vkShader3D = new VKShader(vkInit->GetDevice());
	vkShader3D->LoadShader("../Engine/shaders/spirv/3D.vert.spv", "../Engine/shaders/spirv/3D.frag.spv");

#ifdef _DEBUG
	vkNormal3DShader = new VKShader(vkInit->GetDevice());
	vkNormal3DShader->LoadShader("../Engine/shaders/spirv/Normal3D.vert.spv", "../Engine/shaders/spirv/Normal3D.frag.spv");
#endif

	//2D Pipeline
	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	position_layout.offset = offsetof(TwoDimension::Vertex, position);

	vkPipeline2D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline2D->InitPipeLine(vkShader2D->GetVertexModule(), vkShader2D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(TwoDimension::Vertex), { position_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, false);

	//3D Pipeline
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = offsetof(ThreeDimension::Vertex, position);

	VKAttributeLayout normal_layout;
	normal_layout.vertex_layout_location = 1;
	normal_layout.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	normal_layout.offset = offsetof(ThreeDimension::Vertex, normal);

	VKAttributeLayout uv_layout;
	uv_layout.vertex_layout_location = 2;
	uv_layout.format = VK_FORMAT_R32G32_SFLOAT;
	uv_layout.offset = offsetof(ThreeDimension::Vertex, uv);

	VKAttributeLayout tex_sub_index_layout;
	tex_sub_index_layout.vertex_layout_location = 3;
	tex_sub_index_layout.format = VK_FORMAT_R32_SINT;
	tex_sub_index_layout.offset = offsetof(ThreeDimension::Vertex, texSubIndex);

	vkPipeline3D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3D->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL, true, sizeof(PushConstants), VK_SHADER_STAGE_FRAGMENT_BIT);
	vkPipeline3DLine = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3DLine->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::LINE, true, sizeof(PushConstants), VK_SHADER_STAGE_FRAGMENT_BIT);
#ifdef _DEBUG
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = offsetof(ThreeDimension::NormalVertex, position);

	VKAttributeLayout color_layout;
	color_layout.vertex_layout_location = 1;
	color_layout.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	color_layout.offset = offsetof(ThreeDimension::NormalVertex, color);

	vkPipeline3DNormal = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3DNormal->InitPipeLine(vkNormal3DShader->GetVertexModule(), vkNormal3DShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::NormalVertex), { position_layout, color_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL, false);
#endif

	// Uniform
	uniformBuffer2D.vertexUniformBuffer = std::make_unique<VKUniformBuffer<TwoDimension::VertexUniform>>(vkInit, 1);
	uniformBuffer2D.fragmentUniformBuffer = std::make_unique<VKUniformBuffer<TwoDimension::FragmentUniform>>(vkInit, 1);
	uniformBuffer3D.vertexUniformBuffer = std::make_unique<VKUniformBuffer<ThreeDimension::VertexUniform>>(vkInit, 1);
	uniformBuffer3D.fragmentUniformBuffer = std::make_unique<VKUniformBuffer<ThreeDimension::FragmentUniform>>(vkInit, 1);
	uniformBuffer3D.materialUniformBuffer = std::make_unique<VKUniformBuffer<ThreeDimension::Material>>(vkInit, 1);

	pointLightUniformBuffer = new VKUniformBuffer<ThreeDimension::PointLightUniform>(vkInit, MAX_LIGHT_SIZE);
	directionalLightUniformBuffer = new VKUniformBuffer<ThreeDimension::DirectionalLightUniform>(vkInit, MAX_LIGHT_SIZE);

	imguiManager = new VKImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor->GetDescriptorPool(), &vkRenderPass, msaaSamples);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	VkResult result{ VK_SUCCESS };
	result = vkCreateSampler(*vkInit->GetDevice(), &samplerInfo, nullptr, &immutableSampler);
	const VkDescriptorImageInfo imageInfo
	{
		.sampler = immutableSampler,
		.imageView = VK_NULL_HANDLE,
		.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	imageInfos.resize(500, imageInfo);
}

void VKRenderManager::InitCommandPool()
{
	//Create command pool info
	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = *vkInit->GetQueueFamilyIndex();

	//Create command pool
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateCommandPool(*vkInit->GetDevice(), &commandPoolInfo, nullptr, &vkCommandPool);
		if (result != VK_SUCCESS)
		{
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
				break;
			default:
				break;
			}
			std::cout << '\n';

			throw std::runtime_error{ "Command Pool Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << '\n';
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}
}

void VKRenderManager::InitCommandBuffer()
{
	//Create command buffer info
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = vkCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 2;

	//Create command buffer
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &vkCommandBuffers[0]);
		if (result != VK_SUCCESS)
		{
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
				break;
			default:
				break;
			}
			std::cout << '\n';

			throw std::runtime_error{ "Command Buffer Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << '\n';
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}
}

void VKRenderManager::InitRenderPass()
{
	VkSurfaceFormatKHR surfaceFormat = vkInit->SetSurfaceFormat();

	//Create Attachment Description
	VkAttachmentDescription colorAattachmentDescription{};
	colorAattachmentDescription.format = surfaceFormat.format;
	colorAattachmentDescription.samples = msaaSamples;
	colorAattachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAattachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAattachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAattachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAattachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAattachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachmentDescription{};
	depthAttachmentDescription.format = depthFormat;
	depthAttachmentDescription.samples = msaaSamples;
	depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference depthAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Create Attachment Description
	VkAttachmentDescription colorAattachmentResolveDescription{};
	colorAattachmentResolveDescription.format = surfaceFormat.format;
	colorAattachmentResolveDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAattachmentResolveDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAattachmentResolveDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAattachmentResolveDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAattachmentResolveDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAattachmentResolveDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAattachmentResolveDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentResolveReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentResolveReference.attachment = 2;
	colorAttachmentResolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pResolveAttachments = &colorAttachmentResolveReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

	VkSubpassDependency colorDependency{};
	colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	colorDependency.dstSubpass = 0;
	colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorDependency.srcAccessMask = 0;
	colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	colorDependency.dependencyFlags = 0;

	VkSubpassDependency depthDependency{};
	depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependency.dstSubpass = 0;
	depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	depthDependency.dependencyFlags = 0;

	//Create Renderpass Info
	VkAttachmentDescription attachments[3] = { colorAattachmentDescription, depthAttachmentDescription, colorAattachmentResolveDescription };
	VkSubpassDependency dependencies[2] = { depthDependency, colorDependency };

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 3;
	createInfo.pAttachments = &attachments[0];
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDescription;
	createInfo.dependencyCount = 2;
	createInfo.pDependencies = &dependencies[0];

	//Create Renderpass
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateRenderPass(*vkInit->GetDevice(), &createInfo, nullptr, &vkRenderPass);
		if (result != VK_SUCCESS)
		{
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
				break;
			default:
				break;
			}
			std::cout << '\n';

			throw std::runtime_error{ "RenderPass Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << '\n';
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}
}

void VKRenderManager::InitFrameBuffer(VkExtent2D* swapchainImageExtent_, std::vector<VkImageView>* swapchainImageViews_)
{
	//Allocate memory for framebuffers
	vkFrameBuffers.resize(swapchainImageViews_->size());

	for (int i = 0; i < swapchainImageViews_->size(); ++i)
	{
		VkImageView attachments[3] = { colorImageView, depthImageView, (*swapchainImageViews_)[i] };

		//Create framebuffer info
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = vkRenderPass;
		createInfo.attachmentCount = 3;
		createInfo.pAttachments = &attachments[0];
		createInfo.width = swapchainImageExtent_->width;
		createInfo.height = swapchainImageExtent_->height;
		createInfo.layers = 1;

		try
		{
			//for (auto i = 0; i != swapchainImageViews_->size(); ++i)
			//{
				//createInfo.pAttachments = &(*swapchainImageViews_)[i];

			//Create framebuffer
			VkResult result{ VK_SUCCESS };
			result = vkCreateFramebuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkFrameBuffers[i]);
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << '\n';
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << '\n';
					break;
				default:
					break;
				}
				std::cout << '\n';

				throw std::runtime_error{ "Framebuffer Creation Failed" };
			}
			//}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
			VKRenderManager::~VKRenderManager();
			std::exit(EXIT_FAILURE);
		}
	}
}

void VKRenderManager::CleanSwapChain()
{
	//Destroy FrameBuffer
	for (auto& framebuffer : vkFrameBuffers)
	{
		vkDestroyFramebuffer(*vkInit->GetDevice(), framebuffer, nullptr);
	}
	//Destroy ImageView
	for (auto& imageView : *vkSwapChain->GetSwapChainImageViews())
	{
		vkDestroyImageView(*vkInit->GetDevice(), imageView, nullptr);
	}
	//Destroy SwapChain
	vkDestroySwapchainKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), nullptr);
}

void VKRenderManager::RecreateSwapChain()
{
	int width = 0, height = 0;
	//while (window_->GetMinimized())
	//{
	//	SDL_PumpEvents();
	//}
	SDL_GL_GetDrawableSize(window, &width, &height);

	vkDeviceWaitIdle(*vkInit->GetDevice());

	//CleanSwapChain();

	//vkSwapChain->InitSwapChain();
	//vkSwapChain->InitSwapChainImage();
	//vkSwapChain->InitSwapChainImageView();

	//Destroy FrameBuffer
	for (auto& framebuffer : vkFrameBuffers)
	{
		vkDestroyFramebuffer(*vkInit->GetDevice(), framebuffer, nullptr);
	}

	//Destroy Depth Buffering
	vkDestroyImageView(*vkInit->GetDevice(), depthImageView, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), depthImageMemory, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), depthImage, nullptr);

	//Destroy MSAA
	vkDestroyImageView(*vkInit->GetDevice(), colorImageView, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), colorImageMemory, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), colorImage, nullptr);

	delete vkSwapChain;
	vkSwapChain = new VKSwapChain(vkInit, &vkCommandPool);
	CreateDepthBuffer();
	CreateColorResources();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());
}

void VKRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
	VKTexture* texture = new VKTexture(vkInit, &vkCommandPool);
	texture->LoadTexture(false, path_, name_, flip);

	//vkDestroySampler(*vkInit->GetDevice(), imageInfos[textures.size()].sampler, nullptr);
	textures.push_back(texture);

	int texId = static_cast<int>(textures.size() - 1);
	textures.at(texId)->SetTextureID(texId);

	imageInfos[texId].sampler = *textures[texId]->GetSampler();
	imageInfos[texId].imageView = *textures[texId]->GetImageView();
	imageInfos[texId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void VKRenderManager::DeleteWithIndex(int /*id*/)
{
	//Destroy Texture
	for (auto t : textures)
		delete t;

	//Destroy Batch ImageInfo
	textures.erase(textures.begin(), textures.end());
	imageInfos.erase(imageInfos.begin(), imageInfos.end());

	const VkDescriptorImageInfo imageInfo
	{
		.sampler = immutableSampler,
		.imageView = VK_NULL_HANDLE,
		.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	imageInfos.resize(500, imageInfo);
}

VKTexture* VKRenderManager::GetTexture(std::string name)
{
	/*auto tex = std::find_if(textures.begin(), textures.end(),
		[&name](const VKTexture& texture) {
		return texture.GetName() == name;});
	return const_cast<VKTexture*>(*tex);*/
	for (auto& tex : textures)
	{
		if (tex->GetName() == name)
		{
			return tex;
		}
	}
	return nullptr;
}

void VKRenderManager::LoadSkybox(const std::filesystem::path& path)
{
	if (skyboxEnabled) return;

	skyboxShader = new VKShader(vkInit->GetDevice());
	std::cout << '\n';
	skyboxShader->LoadShader("../Engine/shaders/spirv/Skybox.vert.spv", "../Engine/shaders/spirv/Skybox.frag.spv");

	std::vector<glm::vec3> skyboxVertices = {
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{1.0f, -1.0f, -1.0f },
		{1.0f, -1.0f, -1.0f},
		{1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},

		{-1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},

		{1.0f, -1.0f, -1.0f},
		{1.0f, -1.0f,  1.0f},
		{1.0f,  1.0f,  1.0f},
		{1.0f,  1.0f,  1.0f},
		{1.0f,  1.0f, -1.0f},
		{1.0f, -1.0f, -1.0f},

		{-1.0f, -1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},

		{-1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},

		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f}
	};
	skyboxVertexBuffer = new VKVertexBuffer(vkInit, sizeof(glm::vec3) * skyboxVertices.size(), skyboxVertices.data());

	VKDescriptorLayout fragmentLayout;
	fragmentLayout.descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout.descriptorCount = 1;
	skyboxDescriptor = new VKDescriptor(vkInit, {}, { fragmentLayout });

	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = 0;

	vkPipeline3DSkybox = new VKPipeLine(vkInit->GetDevice(), skyboxDescriptor->GetDescriptorSetLayout());
	vkPipeline3DSkybox->InitPipeLine(skyboxShader->GetVertexModule(), skyboxShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(float) * 3, { position_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, true, sizeof(glm::mat4) * 2, VK_SHADER_STAGE_VERTEX_BIT);

	skybox = new VKSkybox(path, vkInit, &vkCommandPool);

	skyboxEnabled = true;
}

void VKRenderManager::DeleteSkybox()
{
	delete skybox;
	delete skyboxShader;
	delete skyboxDescriptor;
	delete vkPipeline3DSkybox;
	delete skyboxVertexBuffer;
	skyboxEnabled = false;
}

bool VKRenderManager::BeginRender(glm::vec3 bgColor)
{
	isRecreated = false;
	vkSemaphores = (*vkSwapChain->GetSemaphores())[frameIndex];

	//Get image index from swapchain
	//uint32_t swapchainIndex;
	VkResult result = vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, vkSemaphores[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
		isRecreated = true;
		return false;
	}
	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	//	throw std::runtime_error("Failed Acquring SwapChain Image");

	//Get swapchain image
	//VkImage swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
	swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];

	currentFence = &(*vkSwapChain->GetFences())[frameIndex];

	//Wait for fence to be signaled
	if (vkGetFenceStatus(*vkInit->GetDevice(), *currentFence) == VK_NOT_READY)
		vkWaitForFences(*vkInit->GetDevice(), 1, currentFence, VK_TRUE, UINT64_MAX);

	//Set fence to unsignaled
	vkResetFences(*vkInit->GetDevice(), 1, currentFence);

	//--------------------Descriptor Update--------------------//

	std::vector<Sprite*> sprites = Engine::Instance().GetSpriteManager().GetSprites();
	switch (rMode)
	{
	case RenderType::TwoDimension:
		//for (auto& sprite : sprites)
		{
			auto& vertexUniformBuffer = uniformBuffer2D.vertexUniformBuffer;
			currentVertexDescriptorSet = &(*vkDescriptor->GetVertexDescriptorSets())[frameIndex];
			{
				//Create Vertex Material DescriptorBuffer Info
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*vertexUniformBuffer->GetUniformBuffers())[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(TwoDimension::VertexUniform) * sprites.size();

				//Define which resource descriptor set will point
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *currentVertexDescriptorSet;
				descriptorWrite.dstBinding = 0;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorWrite.pBufferInfo = &bufferInfo;

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);

				// @TODO Find a place to update uniform
				//vertexUniformBuffer->UpdateUniform(1, &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform, frameIndex);
			}

			auto& fragmentUniformBuffer = uniformBuffer2D.fragmentUniformBuffer;
			currentFragmentDescriptorSet = &(*vkDescriptor->GetFragmentDescriptorSets())[frameIndex];
			{
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*fragmentUniformBuffer->GetUniformBuffers())[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(TwoDimension::FragmentUniform) * sprites.size();

				VkWriteDescriptorSet descriptorWrite[2] = {};
				descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[0].dstSet = *currentFragmentDescriptorSet;
				descriptorWrite[0].dstBinding = 0;
				descriptorWrite[0].descriptorCount = 1;
				descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorWrite[0].pBufferInfo = &bufferInfo;

				//Define which resource descriptor set will point
				descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[1].dstSet = *currentFragmentDescriptorSet;
				descriptorWrite[1].dstBinding = 1;
				descriptorWrite[1].descriptorCount = 500;
				descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite[1].pImageInfo = imageInfos.data();

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 2, descriptorWrite, 0, nullptr);

				// @TODO Find a place to update uniform
				//fragmentUniformBuffer->UpdateUniform(1, &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform, frameIndex);
			}
		}
		break;
	case RenderType::ThreeDimension:
		//for (auto& sprite : sprites)
		{
			auto& vertexUniformBuffer = uniformBuffer3D.vertexUniformBuffer;
			currentVertexDescriptorSet = &(*vkDescriptor->GetVertexDescriptorSets())[frameIndex];
			{
				//Create Vertex Material DescriptorBuffer Info
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*vertexUniformBuffer->GetUniformBuffers())[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(ThreeDimension::VertexUniform) * sprites.size();

				//Define which resource descriptor set will point
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *currentVertexDescriptorSet;
				descriptorWrite.dstBinding = 0;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorWrite.pBufferInfo = &bufferInfo;

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);

				// @TODO Find a place to update uniform
				//vertexUniformBuffer->UpdateUniform(1, &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform, frameIndex);
			}

			auto& fragmentUniformBuffer = uniformBuffer3D.fragmentUniformBuffer;
			auto& materialUniformBuffer = uniformBuffer3D.materialUniformBuffer;
			//auto& materialUniformBuffer = std::get<VKUniformBuffer<ThreeDimension::Material>*>(sprite->GetMaterialUniformBuffer()->buffer);
			currentFragmentDescriptorSet = &(*vkDescriptor->GetFragmentDescriptorSets())[frameIndex];
			{
				std::vector<VkWriteDescriptorSet> descriptorWrites;

				VkDescriptorBufferInfo fragmentBufferInfo{};
				fragmentBufferInfo.buffer = (*fragmentUniformBuffer->GetUniformBuffers())[frameIndex];
				fragmentBufferInfo.offset = 0;
				fragmentBufferInfo.range = sizeof(ThreeDimension::FragmentUniform) * sprites.size();

				VkWriteDescriptorSet fragmentDescriptorWrite{};
				fragmentDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				fragmentDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
				fragmentDescriptorWrite.dstBinding = 0;
				fragmentDescriptorWrite.descriptorCount = 1;
				fragmentDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				fragmentDescriptorWrite.pBufferInfo = &fragmentBufferInfo;
				descriptorWrites.push_back(fragmentDescriptorWrite);

				//Define which resource descriptor set will point
				VkWriteDescriptorSet textureDescriptorWrite{};
				textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				textureDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
				textureDescriptorWrite.dstBinding = 1;
				textureDescriptorWrite.descriptorCount = 500;
				textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				textureDescriptorWrite.pImageInfo = imageInfos.data();
				descriptorWrites.push_back(textureDescriptorWrite);

				VkDescriptorBufferInfo materialBufferInfo{};
				materialBufferInfo.buffer = (*materialUniformBuffer->GetUniformBuffers())[frameIndex];
				materialBufferInfo.offset = 0;
				materialBufferInfo.range = sizeof(ThreeDimension::Material) * sprites.size();

				VkWriteDescriptorSet materialDescriptorWrite{};
				materialDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				materialDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
				materialDescriptorWrite.dstBinding = 2;
				materialDescriptorWrite.descriptorCount = 1;
				materialDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				materialDescriptorWrite.pBufferInfo = &materialBufferInfo;
				descriptorWrites.push_back(materialDescriptorWrite);

				VkDescriptorBufferInfo directionalLightBufferInfo{};
				if (!directionalLightUniforms.empty())
				{
					directionalLightBufferInfo.buffer = (*(directionalLightUniformBuffer->GetUniformBuffers()))[frameIndex];
					directionalLightBufferInfo.offset = 0;
					directionalLightBufferInfo.range = sizeof(ThreeDimension::DirectionalLightUniform) * directionalLightUniforms.size();

					VkWriteDescriptorSet directionalLightDescriptorWrite{};
					directionalLightDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					directionalLightDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
					directionalLightDescriptorWrite.dstBinding = 3;
					directionalLightDescriptorWrite.descriptorCount = 1;
					directionalLightDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					directionalLightDescriptorWrite.pBufferInfo = &directionalLightBufferInfo;
					descriptorWrites.push_back(directionalLightDescriptorWrite);
				}

				VkDescriptorBufferInfo pointLightBufferInfo{};
				if (!pointLightUniforms.empty())
				{
					pointLightBufferInfo.buffer = (*(pointLightUniformBuffer->GetUniformBuffers()))[frameIndex];
					pointLightBufferInfo.offset = 0;
					pointLightBufferInfo.range = sizeof(ThreeDimension::PointLightUniform) * pointLightUniforms.size();

					VkWriteDescriptorSet pointLightDescriptorWrite{};
					pointLightDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					pointLightDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
					pointLightDescriptorWrite.dstBinding = 4;
					pointLightDescriptorWrite.descriptorCount = 1;
					pointLightDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					pointLightDescriptorWrite.pBufferInfo = &pointLightBufferInfo;
					descriptorWrites.push_back(pointLightDescriptorWrite);
				}

				VkDescriptorImageInfo irradianceImageInfo{};
				irradianceImageInfo.sampler = *skybox->GetIrradiance().first;
				irradianceImageInfo.imageView = *skybox->GetIrradiance().second;
				irradianceImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet irradianceDescriptorWrite{};
				irradianceDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				irradianceDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
				irradianceDescriptorWrite.dstBinding = 5;
				irradianceDescriptorWrite.descriptorCount = 1;
				irradianceDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				irradianceDescriptorWrite.pImageInfo = &irradianceImageInfo;
				descriptorWrites.push_back(irradianceDescriptorWrite);

				VkDescriptorImageInfo prefilterImageInfo{};
				prefilterImageInfo.sampler = *skybox->GetPrefilter().first;
				prefilterImageInfo.imageView = *skybox->GetPrefilter().second;
				prefilterImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet prefilterDescriptorWrite{};
				prefilterDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				prefilterDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
				prefilterDescriptorWrite.dstBinding = 6;
				prefilterDescriptorWrite.descriptorCount = 1;
				prefilterDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				prefilterDescriptorWrite.pImageInfo = &prefilterImageInfo;
				descriptorWrites.push_back(prefilterDescriptorWrite);

				VkDescriptorImageInfo brdfImageInfo{};
				brdfImageInfo.sampler = *skybox->GetBRDF().first;
				brdfImageInfo.imageView = *skybox->GetBRDF().second;
				brdfImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet brdfDescriptorWrite{};
				brdfDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				brdfDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
				brdfDescriptorWrite.dstBinding = 7;
				brdfDescriptorWrite.descriptorCount = 1;
				brdfDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				brdfDescriptorWrite.pImageInfo = &brdfImageInfo;
				descriptorWrites.push_back(brdfDescriptorWrite);

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				// @TODO Find a place to update uniform
				//fragmentUniformBuffer->UpdateUniform(1, &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform, frameIndex);
				//materialUniformBuffer->UpdateUniform(1, &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().material, frameIndex);
				if (!directionalLightUniforms.empty())
					directionalLightUniformBuffer->UpdateUniform(directionalLightUniforms.size(), directionalLightUniforms.data(), frameIndex);
				if (!pointLightUniforms.empty())
					pointLightUniformBuffer->UpdateUniform(pointLightUniforms.size(), pointLightUniforms.data(), frameIndex);
			}
		}

		if (skyboxEnabled)
		{
			//Skybox Fragment Descriptor
			currentFragmentSkyboxDescriptorSet = &(*skyboxDescriptor->GetFragmentDescriptorSets())[frameIndex];
			{
				VkWriteDescriptorSet descriptorWrite{};

				VkDescriptorImageInfo skyboxDescriptorImageInfo{};
				skyboxDescriptorImageInfo.sampler = *skybox->GetCubeMap().first;
				skyboxDescriptorImageInfo.imageView = *skybox->GetCubeMap().second;
				skyboxDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				//Define which resource descriptor set will point
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *currentFragmentSkyboxDescriptorSet;
				descriptorWrite.dstBinding = 0;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.pImageInfo = &skyboxDescriptorImageInfo;

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
		break;
	}

	//--------------------Descriptor Update End--------------------//

	currentCommandBuffer = &vkCommandBuffers[frameIndex];

	//Reset command buffer
	vkResetCommandBuffer(*currentCommandBuffer, 0);

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	vkBeginCommandBuffer(*currentCommandBuffer, &beginInfo);

	//Change image layout to TRANSFER_DST_OPTIMAL
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
		barrier.image = swapchainImage;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		//Record pipeline barrier for chainging image layout
		vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	//Set clear color
	VkClearValue clearValues[2];
	clearValues[0].color = { {bgColor.r, bgColor.g, bgColor.b, 1.f} };
	clearValues[1].depthStencil = { 1.f, 0 };

	//Create RenderPass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = vkRenderPass;
	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearValues;

	//Begin RenderPass
	vkCmdBeginRenderPass(*currentCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//--------------------Begin Draw--------------------//

	//Create Viewport and Scissor for Dynamic State
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(vkSwapChain->GetSwapChainImageExtent()->width);
	viewport.height = static_cast<float>(vkSwapChain->GetSwapChainImageExtent()->height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = *vkSwapChain->GetSwapChainImageExtent();

	//Draw Quad
	VkDeviceSize vertexBufferOffset{ 0 };

	switch (rMode)
	{
	case RenderType::TwoDimension:
		for (int i = 0; i < sprites.size(); ++i)
		{
			auto& buffer = sprites[i]->GetBufferWrapper()->GetBuffer<BufferWrapper::VKBuffer>();
			//VkBuffer* vertexBuffer = std::get<VertexBufferWrapper::VKBuffer>(sprite->GetVertexBuffer()->buffer).vertexBuffer->GetVertexBuffer();
			//VkBuffer* indexBuffer = std::get<VKIndexBuffer*>(sprite->GetIndexBuffer()->buffer)->GetIndexBuffer();
			//Bind Vertex Buffer
			vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, buffer.vertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
			//Bind Index Buffer
			vkCmdBindIndexBuffer(*currentCommandBuffer, *buffer.indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			//Bind Pipeline
			vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLine());
			//Dynamic Viewport & Scissor
			vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
			//Bind Material DescriptorSet
			size_t alignment = vkInit->GetMinUniformBufferOffsetAlignment();
			size_t uniformSize = sizeof(TwoDimension::VertexUniform);
			uint32_t dynamicOffset = static_cast<uint32_t>((i * uniformSize + alignment - 1) & ~(alignment - 1));

			TwoDimension::VertexUniform* vertexDest = (TwoDimension::VertexUniform*)((uint8_t*)mappedMemory + dynamicOffset);
			*vertexDest = sprites[i]->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;

			vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLineLayout(), 0, 1, currentVertexDescriptorSet, 1, &dynamicOffset);
			//Bind Fragment DescriptorSet
			uniformSize = sizeof(TwoDimension::FragmentUniform);
			dynamicOffset = static_cast<uint32_t>((i * uniformSize + alignment - 1) & ~(alignment - 1));

			TwoDimension::FragmentUniform* fragmentDest = (TwoDimension::FragmentUniform*)((uint8_t*)mappedMemory + dynamicOffset);
			*fragmentDest = sprites[i]->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

			vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLineLayout(), 1, 1, currentFragmentDescriptorSet, 0, nullptr);
			//Change Primitive Topology
			//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			//Draw
			vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(sprites[i]->GetBufferWrapper()->GetIndices().size()), 1, 0, 0, 0);
		}
		break;
	case RenderType::ThreeDimension:
		for (auto& sprite : sprites)
		{
			auto& buffer = sprite->GetBufferWrapper()->GetBuffer<BufferWrapper::VKBuffer>();
			//VkBuffer* vertexBuffer = std::get<VertexBufferWrapper::VKBuffer>(sprite->GetVertexBuffer()->buffer).vertexBuffer->GetVertexBuffer();
			//VkBuffer* indexBuffer = std::get<VKIndexBuffer*>(sprite->GetIndexBuffer()->buffer)->GetIndexBuffer();
			switch (pMode)
			{
			case PolygonType::FILL:
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, buffer.vertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
				//Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *buffer.indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				//Bind Pipeline
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3D->GetPipeLine());
				//Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				//Bind Material DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3D->GetPipeLineLayout(), 0, 1, currentVertexDescriptorSet, 0, nullptr);
				//Bind Texture DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3D->GetPipeLineLayout(), 1, 1, currentFragmentDescriptorSet, 0, nullptr);
				//Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				//Push Constant Active Lights
				pushConstants.activeDirectionalLight = static_cast<int>(directionalLightUniforms.size());
				pushConstants.activePointLight = static_cast<int>(pointLightUniforms.size());
				vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3D->GetPipeLineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(sprite->GetBufferWrapper()->GetIndices().size()), 1, 0, 0, 0);
				break;
			case PolygonType::LINE:
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, buffer.vertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
				//Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *buffer.indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				//Bind Pipeline
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DLine->GetPipeLine());
				//Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				//Bind Material DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DLine->GetPipeLineLayout(), 0, 1, currentVertexDescriptorSet, 0, nullptr);
				//Bind Texture DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DLine->GetPipeLineLayout(), 1, 1, currentFragmentDescriptorSet, 0, nullptr);
				//Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
				//Push Constant Active Lights
				//vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3D->GetPipeLineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int) * 2, &activeLights[0]);
				vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3D->GetPipeLineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(sprite->GetBufferWrapper()->GetIndices().size()), 1, 0, 0, 0);
				break;
			}
#ifdef _DEBUG
			if (isDrawNormals)
			{
				VkBuffer* normalVertexBuffer = buffer.normalVertexBuffer->GetVertexBuffer();
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, normalVertexBuffer, &vertexBufferOffset);
				//Bind Index Buffer
				//vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
				//Bind Pipeline
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DNormal->GetPipeLine());
				//Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				//Bind Material DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DNormal->GetPipeLineLayout(), 0, 1, currentVertexDescriptorSet, 0, nullptr);
				//Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				//Draw
				//vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				vkCmdDraw(*currentCommandBuffer, static_cast<uint32_t>(sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices.size()), 1, 0, 0);
			}
#endif
		}

		if (skyboxEnabled)
		{
			//Skybox
			//Bind Vertex Buffer
			vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, skyboxVertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
			//Bind Pipeline
			vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DSkybox->GetPipeLine());
			//Dynamic Viewport & Scissor
			vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
			//Bind Texture DescriptorSet
			vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DSkybox->GetPipeLineLayout(), 0, 1, currentFragmentSkyboxDescriptorSet, 0, nullptr);
			//Change Primitive Topology
			//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			//Push Constant World-To_NDC
			glm::mat4 transform[2] = { Engine::GetCameraManager().GetViewMatrix(), Engine::GetCameraManager().GetProjectionMatrix() };
			vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3DSkybox->GetPipeLineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) * 2, &transform[0]);
			//Draw
			//vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			vkCmdDraw(*currentCommandBuffer, 36, 1, 0, 0);
		}

		break;
	}

	imguiManager->Begin();

	return true;
}

void VKRenderManager::EndRender()
{
	if (!isRecreated)
	{
		imguiManager->End(frameIndex);

		//--------------------End Draw--------------------//

		//End renderpass
		vkCmdEndRenderPass(*currentCommandBuffer);

		//Change image layout to PRESENT_SRC_KHR
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = 0;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
			barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
			barrier.image = swapchainImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;

			//Record pipeline barrier for chainging image layout
			vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		//End command buffer
		vkEndCommandBuffer(*currentCommandBuffer);

		//Create submit info
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//Wait until swapchain image is ready after calculating pixel's result
		//Define pipeline stage that semaphore must be signaled
		constexpr VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &vkSemaphores[IMAGE_AVAILABLE_INDEX];
		submitInfo.pWaitDstStageMask = &waitDstStageMask;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = currentCommandBuffer;
		//submitInfo.pCommandBuffers = &(*imguiManager->GetCommandBuffers())[frameIndex];

		//Define semaphore that informs when command buffer is processed
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &vkSemaphores[RENDERING_DONE_INDEX];

		//Submit queue to command buffer
		vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *currentFence);

		//Wait until all submitted command buffers are handled
		vkDeviceWaitIdle(*vkInit->GetDevice());

		//Create present info
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		//Define semaphore that waits to ensure command buffer to be processed
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &vkSemaphores[RENDERING_DONE_INDEX];

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = vkSwapChain->GetSwapChain();
		presentInfo.pImageIndices = &swapchainIndex;

		//Render image on screen
		VkResult result2 = vkQueuePresentKHR(*vkInit->GetQueue(), &presentInfo);
		if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapChain();
			return;
		}
		//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		//	throw std::runtime_error("Failed Acquring SwapChain Image");

		frameIndex = ++frameIndex % *vkSwapChain->GetBufferCount();
	}
}
