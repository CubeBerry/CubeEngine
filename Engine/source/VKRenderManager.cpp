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
	delete vertex2DBuffer;
	delete vertex3DBuffer;
	delete indexBuffer;
	delete vertexUniform2D;
	delete fragmentUniform2D;
	delete vertexUniform3D;
	delete fragmentUniform3D;
	delete fragmentMaterialUniformBuffer;
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
	delete normalVertexBuffer;
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
	vertexLayout.descriptorType = VKDescriptorLayout::UNIFORM;
	vertexLayout.descriptorCount = 1;

	VKDescriptorLayout fragmentLayout[8];
	fragmentLayout[0].descriptorType = VKDescriptorLayout::UNIFORM;
	fragmentLayout[0].descriptorCount = 1;
	fragmentLayout[1].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[1].descriptorCount = 500;
	fragmentLayout[2].descriptorType = VKDescriptorLayout::UNIFORM;
	fragmentLayout[2].descriptorCount = 1;
	fragmentLayout[3].descriptorType = VKDescriptorLayout::UNIFORM;
	fragmentLayout[3].descriptorCount = 1;
	fragmentLayout[4].descriptorType = VKDescriptorLayout::UNIFORM;
	fragmentLayout[4].descriptorCount = 1;
	fragmentLayout[5].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[5].descriptorCount = 1;
	fragmentLayout[6].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[6].descriptorCount = 1;
	fragmentLayout[7].descriptorType = VKDescriptorLayout::SAMPLER;
	fragmentLayout[7].descriptorCount = 1;
	vkDescriptor = new VKDescriptor(vkInit, { vertexLayout }, { fragmentLayout[0], fragmentLayout[1], fragmentLayout[2], fragmentLayout[3], fragmentLayout[4], fragmentLayout[5], fragmentLayout[6], fragmentLayout[7]});

	vkShader2D = new VKShader(vkInit->GetDevice());
	vkShader2D->LoadShader("../Engine/shader/2D.vert", "../Engine/shader/2D.frag");
	std::cout << '\n';

	vkShader3D = new VKShader(vkInit->GetDevice());
	vkShader3D->LoadShader("../Engine/shader/3D.vert", "../Engine/shader/3D.frag");
	std::cout << '\n';

#ifdef _DEBUG
	vkNormal3DShader = new VKShader(vkInit->GetDevice());
	vkNormal3DShader->LoadShader("../Engine/shader/Normal3D.vert", "../Engine/shader/Normal3D.frag");
	std::cout << '\n';
#endif

	//2D Pipeline
	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	position_layout.offset = offsetof(TwoDimension::Vertex, position);

	VKAttributeLayout index_layout;
	index_layout.vertex_layout_location = 1;
	index_layout.format = VK_FORMAT_R32_SINT;
	index_layout.offset = offsetof(TwoDimension::Vertex, index);

	vkPipeline2D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline2D->InitPipeLine(vkShader2D->GetVertexModule(), vkShader2D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(TwoDimension::Vertex), { position_layout, index_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, false);

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

	index_layout.vertex_layout_location = 3;
	index_layout.format = VK_FORMAT_R32_SINT;
	index_layout.offset = offsetof(ThreeDimension::Vertex, index);

	VKAttributeLayout tex_sub_index_layout;
	tex_sub_index_layout.vertex_layout_location = 4;
	tex_sub_index_layout.format = VK_FORMAT_R32_SINT;
	tex_sub_index_layout.offset = offsetof(ThreeDimension::Vertex, texSubIndex);

	vkPipeline3D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3D->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, index_layout, tex_sub_index_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL, true, sizeof(int) * 2, VK_SHADER_STAGE_FRAGMENT_BIT);
	vkPipeline3DLine = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3DLine->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, index_layout, tex_sub_index_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::LINE, true, sizeof(int) * 2, VK_SHADER_STAGE_FRAGMENT_BIT);
#ifdef _DEBUG
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = offsetof(ThreeDimension::NormalVertex, position);

	VKAttributeLayout color_layout;
	color_layout.vertex_layout_location = 1;
	color_layout.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	color_layout.offset = offsetof(ThreeDimension::NormalVertex, color);

	index_layout.vertex_layout_location = 2;
	index_layout.format = VK_FORMAT_R32_SINT;
	index_layout.offset = offsetof(ThreeDimension::NormalVertex, index);

	vkPipeline3DNormal = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3DNormal->InitPipeLine(vkNormal3DShader->GetVertexModule(), vkNormal3DShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::NormalVertex), { position_layout, color_layout, index_layout }, msaaSamples, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL, false);
#endif

	vertexUniform2D = new VKUniformBuffer<TwoDimension::VertexUniform>(vkInit, MAX_OBJECT_SIZE);
	fragmentUniform2D = new VKUniformBuffer<TwoDimension::FragmentUniform>(vkInit, MAX_OBJECT_SIZE);

	vertexUniform3D = new VKUniformBuffer<ThreeDimension::VertexUniform>(vkInit, MAX_OBJECT_SIZE);
	fragmentUniform3D = new VKUniformBuffer<ThreeDimension::FragmentUniform>(vkInit, MAX_OBJECT_SIZE);
	fragmentMaterialUniformBuffer = new VKUniformBuffer<ThreeDimension::Material>(vkInit, MAX_OBJECT_SIZE);
	pointLightUniformBuffer = new VKUniformBuffer<ThreeDimension::PointLightUniform>(vkInit, MAX_LIGHT_SIZE);
	directionalLightUniformBuffer = new VKUniformBuffer<ThreeDimension::DirectionalLightUniform>(vkInit, MAX_LIGHT_SIZE);

	imguiManager = new VKImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor->GetDescriptorPool(), &vkRenderPass, msaaSamples);

	//for (int i = 0; i < 500; ++i)
	//{
	//	VkSamplerCreateInfo samplerInfo{};
	//	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	//	VkSampler immutableSampler;
	//	VkResult result{ VK_SUCCESS };
	//	result = vkCreateSampler(*vkInit->GetDevice(), &samplerInfo, nullptr, &immutableSampler);

	//	VkDescriptorImageInfo imageInfo{};
	//	imageInfo.sampler = immutableSampler;
	//	imageInfo.imageView = VK_NULL_HANDLE;
	//	imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	imageInfos.push_back(imageInfo);
	//}

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

void VKRenderManager::LoadQuad(glm::vec4 color_, float isTex_, float isTexel_)
{
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(-1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(1.f, -1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(-1.f, -1.f, 1.f), quadCount));
	if (vertex2DBuffer != nullptr)
		delete vertex2DBuffer;
	vertex2DBuffer = new VKVertexBuffer<TwoDimension::Vertex>(vkInit, &vertices2D);

	uint64_t indexNumber{ vertices2D.size() / 4 - 1 };
	indices.push_back(static_cast<uint32_t>(4 * indexNumber));
	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 1));
	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 2));
	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 2));
	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 3));
	indices.push_back(static_cast<uint32_t>(4 * indexNumber));
	if (indexBuffer != nullptr)
		delete indexBuffer;
	indexBuffer = new VKIndexBuffer(vkInit, &vkCommandPool, &indices);

	quadCount++;

	TwoDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexUniforms2D.push_back(mat);
	vertexUniforms2D.back().color = color_;
	vertexUniforms2D.back().isTex = isTex_;
	vertexUniforms2D.back().isTexel = isTexel_;

	TwoDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragUniforms2D.push_back(tIndex);

	//if (vertexUniform2D != nullptr)
	//	delete vertexUniform2D;
	//vertexUniform2D = new VKUniformBuffer<TwoDimension::VertexUniform>(vkInit, quadCount);

	//if (fragmentUniform2D != nullptr)
	//	delete fragmentUniform2D;
	//fragmentUniform2D = new VKUniformBuffer<TwoDimension::FragmentUniform>(vkInit, quadCount);
}

void VKRenderManager::DeleteWithIndex(int id)
{
	quadCount--;

	if (quadCount == 0)
	{
		switch (rMode)
		{
		case RenderType::TwoDimension:
			vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
			delete vertex2DBuffer;
			vertex2DBuffer = nullptr;
			break;
		case RenderType::ThreeDimension:
			vertices3D.erase(end(vertices3D) - *verticesPerMesh.begin(), end(vertices3D));
			delete vertex3DBuffer;
			vertex3DBuffer = nullptr;
#ifdef _DEBUG
			normalVertices3D.erase(end(normalVertices3D) - *normalVerticesPerMesh.begin(), end(normalVertices3D));
			delete normalVertexBuffer;
			normalVertexBuffer = nullptr;
#endif
			break;
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			indices.erase(end(indices) - 6, end(indices));
			break;
		case RenderType::ThreeDimension:
			indices.erase(end(indices) - *indicesPerMesh.begin(), end(indices));
			break;
		}
		delete indexBuffer;
		indexBuffer = nullptr;

		if (rMode == RenderType::ThreeDimension)
		{
			verticesPerMesh.erase(verticesPerMesh.begin());
#ifdef _DEBUG
			normalVerticesPerMesh.erase(normalVerticesPerMesh.begin());
#endif
			indicesPerMesh.erase(indicesPerMesh.begin());
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
			//delete vertexUniform2D;
			//vertexUniform2D = nullptr;
			break;
		case RenderType::ThreeDimension:
			vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
			//delete vertexUniform3D;
			//vertexUniform3D = nullptr;
			break;
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			fragUniforms2D.erase(end(fragUniforms2D) - 1);
			//delete fragmentUniform2D;
			//fragmentUniform2D = nullptr;
			break;
		case RenderType::ThreeDimension:
			fragUniforms3D.erase(end(fragUniforms3D) - 1);
			//delete fragmentUniform3D;
			//fragmentUniform3D = nullptr;

			fragMaterialUniforms3D.erase(end(fragMaterialUniforms3D) - 1);
			//delete fragmentMaterialUniformBuffer;
			//fragmentMaterialUniformBuffer = nullptr;
			break;
		}

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

		return;
	}

	//Create Command Buffer Allocate Info
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = vkCommandPool;
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

	switch (rMode)
	{
	case RenderType::TwoDimension:
		vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
		vkCmdUpdateBuffer(commandBuffer, *vertex2DBuffer->GetVertexBuffer(), 0, vertices2D.size() * sizeof(TwoDimension::Vertex), vertices2D.data());

		indices.erase(end(indices) - 6, end(indices));
		vkCmdUpdateBuffer(commandBuffer, *indexBuffer->GetIndexBuffer(), 0, indices.size() * sizeof(uint32_t), indices.data());
		break;
	case RenderType::ThreeDimension:
		unsigned int beginCount{ 0 };
		for (int v = 0; v < id; ++v)
		{
			beginCount += verticesPerMesh[v];
		}
		vertices3D.erase(begin(vertices3D) + beginCount, begin(vertices3D) + beginCount + verticesPerMesh[id]);
		for (auto it = vertices3D.begin() + beginCount; it != vertices3D.end(); ++it)
		{
			it->index--;
		}

		beginCount = 0;
		for (int v = 0; v < id; ++v)
		{
			beginCount += indicesPerMesh[v];
		}
		indices.erase(begin(indices) + beginCount, begin(indices) + beginCount + indicesPerMesh[id]);
		for (auto it = indices.begin() + beginCount; it != indices.end(); ++it)
		{
			(*it) = (*it) - static_cast<unsigned short>(verticesPerMesh[id]);
		}
		//vkCmdUpdateBuffer(commandBuffer, *vertex3DBuffer->GetVertexBuffer(), 0, vertices3D.size() * sizeof(ThreeDimension::Vertex), vertices3D.data());
		vertex3DBuffer->UpdateVertexBuffer(&vertices3D);
		//vkCmdUpdateBuffer(commandBuffer, *indexBuffer->GetIndexBuffer(), 0, indices.size() * sizeof(uint32_t), indices.data());
		indexBuffer->UpdateIndexBuffer(&indices);

#ifdef _DEBUG
		beginCount = 0;
		for (int vn = 0; vn < id; ++vn)
		{
			beginCount += normalVerticesPerMesh[vn];
		}

		normalVertices3D.erase(begin(normalVertices3D) + beginCount, begin(normalVertices3D) + beginCount + normalVerticesPerMesh[id]);
		for (auto it = normalVertices3D.begin() + beginCount; it != normalVertices3D.end(); ++it)
		{
			it->index--;
		}
		//vkCmdUpdateBuffer(commandBuffer, *normalVertexBuffer->GetVertexBuffer(), 0, normalVertices3D.size() * sizeof(ThreeDimension::NormalVertex), normalVertices3D.data());
		normalVertexBuffer->UpdateVertexBuffer(&normalVertices3D);
#endif
		break;
	}
	//vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
	//vkCmdUpdateBuffer(commandBuffer, *vertex2DBuffer->GetVertexBuffer(), 0, vertices2D.size() * sizeof(TwoDimension::Vertex), vertices2D.data());

	//indices.erase(end(indices) - 6, end(indices));
	//vkCmdUpdateBuffer(commandBuffer, *indexBuffer->GetIndexBuffer(), 0, indices.size() * sizeof(uint32_t), indices.data());

	if (rMode == RenderType::ThreeDimension)
	{
		verticesPerMesh.erase(verticesPerMesh.begin() + id);
		indicesPerMesh.erase(indicesPerMesh.begin() + id);
#ifdef _DEBUG
		normalVerticesPerMesh.erase(normalVerticesPerMesh.begin() + id);
#endif
	}

	switch (rMode)
	{
	case RenderType::TwoDimension:
		vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
		for (auto u : *vertexUniform2D->GetUniformBuffers())
		{
			vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(TwoDimension::VertexUniform), vertexUniforms2D.data());
		}
		break;
	case RenderType::ThreeDimension:
		vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
		for (auto u : *vertexUniform3D->GetUniformBuffers())
		{
			vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(ThreeDimension::VertexUniform), vertexUniforms3D.data());
		}
		break;
	}

	switch (rMode)
	{
	case RenderType::TwoDimension:
		fragUniforms2D.erase(end(fragUniforms2D) - 1);
		for (auto u : *fragmentUniform2D->GetUniformBuffers())
		{
			vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(TwoDimension::FragmentUniform), fragUniforms2D.data());
		}
		break;
	case RenderType::ThreeDimension:
		fragUniforms3D.erase(end(fragUniforms3D) - 1);
		for (auto u : *fragmentUniform3D->GetUniformBuffers())
		{
			vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(ThreeDimension::FragmentUniform), fragUniforms3D.data());
		}

		fragMaterialUniforms3D.erase(end(fragMaterialUniforms3D) - 1);
		for (auto u : *fragmentMaterialUniformBuffer->GetUniformBuffers())
		{
			vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(ThreeDimension::Material), fragMaterialUniforms3D.data());
		}
		break;
	}

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
	vkFreeCommandBuffers(*vkInit->GetDevice(), vkCommandPool, 1, &commandBuffer);
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

void VKRenderManager::LoadMesh(MeshType type, const std::filesystem::path& path, glm::vec4 color, int stacks, int slices, float metallic, float roughness)
{
	CreateMesh(type, path, stacks, slices);

	if (vertex3DBuffer != nullptr)
		delete vertex3DBuffer;
	vertex3DBuffer = new VKVertexBuffer<ThreeDimension::Vertex>(vkInit, &vertices3D);
#ifdef _DEBUG
	if (normalVertexBuffer != nullptr)
		delete normalVertexBuffer;
	normalVertexBuffer = new VKVertexBuffer<ThreeDimension::NormalVertex>(vkInit, &normalVertices3D);
#endif

	if (indexBuffer != nullptr)
		delete indexBuffer;
	indexBuffer = new VKIndexBuffer(vkInit, &vkCommandPool, &indices);

	quadCount++;

	ThreeDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexUniforms3D.push_back(mat);
	vertexUniforms3D.back().color = color;

	ThreeDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragUniforms3D.push_back(tIndex);

	ThreeDimension::Material material;
	material.metallic = metallic;
	material.roughness = roughness;
	fragMaterialUniforms3D.push_back(material);

	//if (vertexUniform3D != nullptr)
	//	delete vertexUniform3D;
	//vertexUniform3D = new VKUniformBuffer<ThreeDimension::VertexUniform>(vkInit, quadCount);

	//if (fragmentUniform3D != nullptr)
	//	delete fragmentUniform3D;
	//fragmentUniform3D = new VKUniformBuffer<ThreeDimension::FragmentUniform>(vkInit, quadCount);

	//if (fragmentMaterialUniformBuffer != nullptr)
	//	delete fragmentMaterialUniformBuffer;
	//fragmentMaterialUniformBuffer = new VKUniformBuffer<ThreeDimension::Material>(vkInit, quadCount);
}

void VKRenderManager::LoadSkybox(const std::filesystem::path& path)
{
	if (skyboxEnabled) return;

	skyboxShader = new VKShader(vkInit->GetDevice());
	std::cout << '\n';
	skyboxShader->LoadShader("../Engine/shader/Skybox.vert", "../Engine/shader/Skybox.frag");

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
	skyboxVertexBuffer = new VKVertexBuffer<glm::vec3>(vkInit, &skyboxVertices);

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

void VKRenderManager::BeginRender(glm::vec3 bgColor)
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
		return;
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

	switch(rMode)
	{
	case RenderType::TwoDimension:
		if (vertexUniform2D != nullptr)
		{
			currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
			{
				//Create Vertex Material DescriptorBuffer Info
				//std::vector<VkDescriptorBufferInfo> bufferInfos;
				//for (auto& t : textures)
				//{
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*(vertexUniform2D->GetUniformBuffers()))[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(TwoDimension::VertexUniform) * quadCount;
				//bufferInfos.push_back(bufferInfo);
				//}

				//Define which resource descriptor set will point
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *currentVertexMaterialDescriptorSet;
				descriptorWrite.dstBinding = 0;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.pBufferInfo = &bufferInfo;

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
			vertexUniform2D->UpdateUniform(vertexUniforms2D.size(), vertexUniforms2D.data(), frameIndex);
		}

		if (fragmentUniform2D != nullptr)
		{
			currentTextureDescriptorSet = &(*vkDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
			{
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*(fragmentUniform2D->GetUniformBuffers()))[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(TwoDimension::FragmentUniform) * quadCount;

				VkWriteDescriptorSet descriptorWrite[2] = {};
				descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[0].dstSet = *currentTextureDescriptorSet;
				descriptorWrite[0].dstBinding = 0;
				descriptorWrite[0].descriptorCount = 1;
				descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite[0].pBufferInfo = &bufferInfo;

				//Create Texture DescriptorBuffer Info
				//for (int i = 0; i < textures.size(); ++i)
				//{
				//	imageInfos[i].sampler = *textures[i]->GetSampler();
				//	imageInfos[i].imageView = *textures[i]->GetImageView();
				//	imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//}

				//Define which resource descriptor set will point
				descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[1].dstSet = *currentTextureDescriptorSet;
				descriptorWrite[1].dstBinding = 1;
				descriptorWrite[1].descriptorCount = 500;
				descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite[1].pImageInfo = imageInfos.data();

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 2, descriptorWrite, 0, nullptr);
			}
			fragmentUniform2D->UpdateUniform(fragUniforms2D.size(), fragUniforms2D.data(), frameIndex);
		}
		break;
	case RenderType::ThreeDimension:
		if (vertexUniform3D != nullptr)
		{
			currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
			{
				//Create Vertex Material DescriptorBuffer Info
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*(vertexUniform3D->GetUniformBuffers()))[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(ThreeDimension::VertexUniform) * quadCount;

				//Define which resource descriptor set will point
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *currentVertexMaterialDescriptorSet;
				descriptorWrite.dstBinding = 0;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.pBufferInfo = &bufferInfo;

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
			vertexUniform3D->UpdateUniform(vertexUniforms3D.size(), vertexUniforms3D.data(), frameIndex);
		}

		if (fragmentUniform3D != nullptr)
		{
			currentTextureDescriptorSet = &(*vkDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
			{
				std::vector<VkWriteDescriptorSet> descriptorWrites;

				VkDescriptorBufferInfo fragmentBufferInfo{};
				fragmentBufferInfo.buffer = (*(fragmentUniform3D->GetUniformBuffers()))[frameIndex];
				fragmentBufferInfo.offset = 0;
				fragmentBufferInfo.range = sizeof(ThreeDimension::FragmentUniform) * quadCount;

				VkWriteDescriptorSet fragmentDescriptorWrite{};
				fragmentDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				fragmentDescriptorWrite.dstSet = *currentTextureDescriptorSet;
				fragmentDescriptorWrite.dstBinding = 0;
				fragmentDescriptorWrite.descriptorCount = 1;
				fragmentDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				fragmentDescriptorWrite.pBufferInfo = &fragmentBufferInfo;
				descriptorWrites.push_back(fragmentDescriptorWrite);

				//Create Texture DescriptorBuffer Info
				//for (int i = 0; i < textures.size(); ++i)
				//{
				//	imageInfos[i].sampler = *textures[i]->GetSampler();
				//	imageInfos[i].imageView = *textures[i]->GetImageView();
				//	imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//}

				//Define which resource descriptor set will point
				VkWriteDescriptorSet textureDescriptorWrite{};
				textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				textureDescriptorWrite.dstSet = *currentTextureDescriptorSet;
				textureDescriptorWrite.dstBinding = 1;
				textureDescriptorWrite.descriptorCount = 500;
				textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				textureDescriptorWrite.pImageInfo = imageInfos.data();
				descriptorWrites.push_back(textureDescriptorWrite);

				VkDescriptorBufferInfo materialBufferInfo{};
				materialBufferInfo.buffer = (*(fragmentMaterialUniformBuffer->GetUniformBuffers()))[frameIndex];
				materialBufferInfo.offset = 0;
				materialBufferInfo.range = sizeof(ThreeDimension::Material) * quadCount;

				VkWriteDescriptorSet materialDescriptorWrite{};
				materialDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				materialDescriptorWrite.dstSet = *currentTextureDescriptorSet;
				materialDescriptorWrite.dstBinding = 2;
				materialDescriptorWrite.descriptorCount = 1;
				materialDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
					directionalLightDescriptorWrite.dstSet = *currentTextureDescriptorSet;
					directionalLightDescriptorWrite.dstBinding = 3;
					directionalLightDescriptorWrite.descriptorCount = 1;
					directionalLightDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
					pointLightDescriptorWrite.dstSet = *currentTextureDescriptorSet;
					pointLightDescriptorWrite.dstBinding = 4;
					pointLightDescriptorWrite.descriptorCount = 1;
					pointLightDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					pointLightDescriptorWrite.pBufferInfo = &pointLightBufferInfo;
					descriptorWrites.push_back(pointLightDescriptorWrite);
				}

				VkDescriptorImageInfo irradianceImageInfo{};
				irradianceImageInfo.sampler = *skybox->GetIrradiance().first;
				irradianceImageInfo.imageView = *skybox->GetIrradiance().second;
				irradianceImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet irradianceDescriptorWrite{};
				irradianceDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				irradianceDescriptorWrite.dstSet = *currentTextureDescriptorSet;
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
				prefilterDescriptorWrite.dstSet = *currentTextureDescriptorSet;
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
				brdfDescriptorWrite.dstSet = *currentTextureDescriptorSet;
				brdfDescriptorWrite.dstBinding = 7;
				brdfDescriptorWrite.descriptorCount = 1;
				brdfDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				brdfDescriptorWrite.pImageInfo = &brdfImageInfo;
				descriptorWrites.push_back(brdfDescriptorWrite);

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
			fragmentUniform3D->UpdateUniform(fragUniforms3D.size(), fragUniforms3D.data(), frameIndex);
			fragmentMaterialUniformBuffer->UpdateUniform(fragMaterialUniforms3D.size(), fragMaterialUniforms3D.data(), frameIndex);
			if (!directionalLightUniforms.empty())
				directionalLightUniformBuffer->UpdateUniform(directionalLightUniforms.size(), directionalLightUniforms.data(), frameIndex);
			if (!pointLightUniforms.empty())
				pointLightUniformBuffer->UpdateUniform(pointLightUniforms.size(), pointLightUniforms.data(), frameIndex);
		}

		if (skyboxEnabled)
		{
			//Skybox Fragment Descriptor
			currentFragmentSkyboxDescriptorSet = &(*skyboxDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
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

	//VkClearValue clearValue{};
	//clearValue.color.float32[0] = bgColor.r;	//R
	//clearValue.color.float32[1] = bgColor.g;	//G
	//clearValue.color.float32[2] = bgColor.b;	//B
	//clearValue.color.float32[3] = bgColor.a;	//A
	//clearValue.depthStencil = { 1.0f, 0 };

	//Create renderpass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = vkRenderPass;
	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearValues;

	//Begin renderpass
	vkCmdBeginRenderPass(*currentCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//--------------------Begin Draw--------------------//

	//Create Viewport and Scissor for Dynamic State
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = static_cast<float>(vkSwapChain->GetSwapChainImageExtent()->height);
	viewport.width = static_cast<float>(vkSwapChain->GetSwapChainImageExtent()->width);
	viewport.height = -static_cast<float>(vkSwapChain->GetSwapChainImageExtent()->height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = *vkSwapChain->GetSwapChainImageExtent();

	//Draw Quad
	VkDeviceSize vertexBufferOffset{ 0 };

	switch(rMode)
	{
	case RenderType::TwoDimension:
		if (vertex2DBuffer != nullptr)
		{
			//Bind Vertex Buffer
			vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, vertex2DBuffer->GetVertexBuffer(), &vertexBufferOffset);
			//Bind Index Buffer
			vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			//Bind Pipeline
			vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLine());
			//Dynamic Viewport & Scissor
			vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
			//Bind Material DescriptorSet
			vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
			//Bind Texture DescriptorSet
			vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
			//Change Primitive Topology
			//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			//Draw
			vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}
		break;
	case RenderType::ThreeDimension:
		if (vertex3DBuffer != nullptr)
		{
			switch (pMode)
			{
			case PolygonType::FILL:
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, vertex3DBuffer->GetVertexBuffer(), &vertexBufferOffset);
				//Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				//Bind Pipeline
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3D->GetPipeLine());
				//Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				//Bind Material DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3D->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
				//Bind Texture DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3D->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
				//Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				//Push Constant Active Lights
				activeLights[0] = static_cast<int>(pointLightUniforms.size());
				activeLights[1] = static_cast<int>(directionalLightUniforms.size());
				vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3D->GetPipeLineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int) * 2, &activeLights[0]);
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				break;
			case PolygonType::LINE:
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, vertex3DBuffer->GetVertexBuffer(), &vertexBufferOffset);
				//Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				//Bind Pipeline
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DLine->GetPipeLine());
				//Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				//Bind Material DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DLine->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
				//Bind Texture DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DLine->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
				//Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
				//Push Constant Active Lights
				vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3D->GetPipeLineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int) * 2, &activeLights[0]);
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				break;
			}
#ifdef _DEBUG
			if (isDrawNormals)
			{
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, normalVertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
				//Bind Index Buffer
				//vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
				//Bind Pipeline
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DNormal->GetPipeLine());
				//Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				//Bind Material DescriptorSet
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DNormal->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
				//Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				//Draw
				//vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				vkCmdDraw(*currentCommandBuffer, static_cast<uint32_t>(normalVertices3D.size()), 1, 0, 0);
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
