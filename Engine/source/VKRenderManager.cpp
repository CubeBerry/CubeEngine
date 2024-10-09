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

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

VKRenderManager::~VKRenderManager()
{
	vkDeviceWaitIdle(*vkInit->GetDevice());

#ifdef _DEBUG
	//delete ImGui
	delete imguiManager;
#endif

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
	delete vertexLightingUniformBuffer;

	//Destroy Texture
	for (const auto t : textures)
		delete t;

	//Destroy Batch ImageInfo
	size_t texSize{ textures.size() };
	for (size_t i = texSize; i < imageInfos.size(); ++i)
		vkDestroySampler(*vkInit->GetDevice(), imageInfos[i].sampler, nullptr);

	textures.erase(textures.begin(), textures.end());
	imageInfos.erase(imageInfos.begin(), imageInfos.end());

	//Destroy Depth Buffering
	vkDestroyImageView(*vkInit->GetDevice(), depthImageView, nullptr);
	vkFreeMemory(*vkInit->GetDevice(), depthImageMemory, nullptr);
	vkDestroyImage(*vkInit->GetDevice(), depthImage, nullptr);

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

void VKRenderManager::Initialize(SDL_Window* window_)
{
	window = window_;

	vkInit = new VKInit;
	vkInit->Initialize(window);

	InitCommandPool();
	InitCommandBuffer();

	vkSwapChain = new VKSwapChain(vkInit, &vkCommandPool);

	//Depth Buffering
	VkFormat depthFormat = FindDepthFormat();

	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = depthFormat;
		createInfo.extent = { vkSwapChain->GetSwapChainImageExtent()->width, vkSwapChain->GetSwapChainImageExtent()->height, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
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
	createInfo.subresourceRange.levelCount = 1;
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
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}

	InitRenderPass();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());

	vkDescriptor = new VKDescriptor(vkInit);

	vkShader2D = new VKShader(vkInit->GetDevice());
	vkShader2D->LoadShader("../Engine/shader/Vulkan2D.vert", "../Engine/shader/Vulkan2D.frag");
	std::cout << std::endl;

	vkShader3D = new VKShader(vkInit->GetDevice());
	vkShader3D->LoadShader("../Engine/shader/Vulkan3D.vert", "../Engine/shader/Vulkan3D.frag");
	std::cout << std::endl;

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
	vkPipeline2D->InitPipeLine(vkShader2D->GetVertexModule(), vkShader2D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(TwoDimension::Vertex), { position_layout, index_layout }, VK_CULL_MODE_NONE, POLYGON_MODE::FILL);

	//3D Pipeline
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32A32_SFLOAT;
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

	vkPipeline3D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3D->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, index_layout }, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL);
	vkPipeline3DLine = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline3DLine->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, index_layout }, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::LINE);

	vertexLightingUniformBuffer = new VKUniformBuffer<ThreeDimension::VertexLightingUniform>(vkInit, 1);

#ifdef _DEBUG
	imguiManager = new VKImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor->GetDescriptorPool(), &vkRenderPass);
#endif

	for (int i = 0; i < 500; ++i)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		VkSampler immutableSampler;
		VkResult result{ VK_SUCCESS };
		result = vkCreateSampler(*vkInit->GetDevice(), &samplerInfo, nullptr, &immutableSampler);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = immutableSampler;
		imageInfo.imageView = VK_NULL_HANDLE;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfos.push_back(imageInfo);
	}
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
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
				break;
			default:
				break;
			}
			std::cout << std::endl;

			throw std::runtime_error{ "Command Pool Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
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
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
	}
}

void VKRenderManager::InitRenderPass()
{
	VkSurfaceFormatKHR surfaceFormat = vkInit->SetSurfaceFormat();

	//Create Attachment Description
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = surfaceFormat.format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference colorAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthDescription{};
	depthDescription.format = FindDepthFormat();
	depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Define which attachment should subpass refernece of renderpass
	VkAttachmentReference depthAttachmentReference{};
	//attachment == Index of VkAttachmentDescription array
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

	//VkSubpassDependency dependency{};
	//dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	//Create Renderpass Info
	std::array<VkAttachmentDescription, 2> attachments = { attachmentDescription, depthDescription };

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDescription;
	//createInfo.dependencyCount = 1;
	//createInfo.pDependencies = &dependency;

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
		std::array<VkImageView, 2> attachments = { (*swapchainImageViews_)[i], depthImageView};

		//Create framebuffer info
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = vkRenderPass;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
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
			//}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
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

	delete vkSwapChain;
	vkSwapChain = new VKSwapChain(vkInit, &vkCommandPool);
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());
}

void VKRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_)
{
	VKTexture* texture = new VKTexture(vkInit, &vkCommandPool);
	texture->LoadTexture(path_, name_);

	vkDestroySampler(*vkInit->GetDevice(), imageInfos[textures.size()].sampler, nullptr);
	textures.push_back(texture);

	int texId = static_cast<int>(textures.size() - 1);
	textures.at(texId)->SetTextureID(texId);
}

void VKRenderManager::LoadQuad(glm::vec4 color_, float isTex_, float isTexel_)
{
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), quadCount));
	if (vertex2DBuffer != nullptr)
		delete vertex2DBuffer;
	vertex2DBuffer = new VKVertexBuffer<TwoDimension::Vertex>(vkInit, &vertices2D);

	uint64_t indexNumber{ vertices2D.size() / 4 - 1 };
	indices.push_back(static_cast<uint16_t>(4 * indexNumber));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 1));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 3));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber));
	if (indexBuffer != nullptr)
		delete indexBuffer;
	indexBuffer = new VKIndexBuffer(vkInit, &vkCommandPool, &indices);

	quadCount++;

	if (vertexUniform2D != nullptr)
		delete vertexUniform2D;
	vertexUniform2D = new VKUniformBuffer<TwoDimension::VertexUniform>(vkInit, quadCount);

	if (fragmentUniform2D != nullptr)
		delete fragmentUniform2D;
	fragmentUniform2D = new VKUniformBuffer<TwoDimension::FragmentUniform>(vkInit, quadCount);

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
			indicesPerMesh.erase(indicesPerMesh.begin());
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
			delete vertexUniform2D;
			vertexUniform2D = nullptr;
			break;
		case RenderType::ThreeDimension:
			vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
			delete vertexUniform3D;
			vertexUniform3D = nullptr;
			break;
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			fragUniforms2D.erase(end(fragUniforms2D) - 1);
			delete fragmentUniform2D;
			fragmentUniform2D = nullptr;
			break;
		case RenderType::ThreeDimension:
			fragUniforms3D.erase(end(fragUniforms3D) - 1);
			delete fragmentUniform3D;
			fragmentUniform3D = nullptr;
			break;
		}

		//Destroy Texture
		for (auto t : textures)
			delete t;

		//Destroy Batch ImageInfo
		size_t texSize{ textures.size() };
		for (size_t i = texSize; i < imageInfos.size(); ++i)
			vkDestroySampler(*vkInit->GetDevice(), imageInfos[i].sampler, nullptr);

		textures.erase(textures.begin(), textures.end());
		imageInfos.erase(imageInfos.begin(), imageInfos.end());

		for (int i = 0; i < 500; ++i)
		{
			VkSamplerCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

			VkSampler immutableSampler;
			VkResult result{ VK_SUCCESS };
			result = vkCreateSampler(*vkInit->GetDevice(), &createInfo, nullptr, &immutableSampler);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.sampler = immutableSampler;
			imageInfo.imageView = VK_NULL_HANDLE;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfos.push_back(imageInfo);
		}

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
		vkCmdUpdateBuffer(commandBuffer, *indexBuffer->GetIndexBuffer(), 0, indices.size() * sizeof(uint16_t), indices.data());
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

		vkCmdUpdateBuffer(commandBuffer, *vertex3DBuffer->GetVertexBuffer(), 0, vertices3D.size() * sizeof(ThreeDimension::Vertex), vertices3D.data());
		vkCmdUpdateBuffer(commandBuffer, *indexBuffer->GetIndexBuffer(), 0, indices.size() * sizeof(uint16_t), indices.data());
		break;
	}
	//vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
	//vkCmdUpdateBuffer(commandBuffer, *vertex2DBuffer->GetVertexBuffer(), 0, vertices2D.size() * sizeof(TwoDimension::Vertex), vertices2D.data());

	//indices.erase(end(indices) - 6, end(indices));
	//vkCmdUpdateBuffer(commandBuffer, *indexBuffer->GetIndexBuffer(), 0, indices.size() * sizeof(uint16_t), indices.data());

	if (rMode == RenderType::ThreeDimension)
	{
		verticesPerMesh.erase(verticesPerMesh.begin() + id);
		indicesPerMesh.erase(indicesPerMesh.begin() + id);
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

void VKRenderManager::LoadMesh(MeshType type, const std::filesystem::path& path, glm::vec4 color, int stacks, int slices)
{
	CreateMesh(type, path, stacks, slices);

	if (vertex3DBuffer != nullptr)
		delete vertex3DBuffer;
	vertex3DBuffer = new VKVertexBuffer<ThreeDimension::Vertex>(vkInit, &vertices3D);

	if (indexBuffer != nullptr)
		delete indexBuffer;
	indexBuffer = new VKIndexBuffer(vkInit, &vkCommandPool, &indices);

	quadCount++;

	if (vertexUniform3D != nullptr)
		delete vertexUniform3D;
	vertexUniform3D = new VKUniformBuffer<ThreeDimension::VertexUniform>(vkInit, quadCount);

	if (fragmentUniform3D != nullptr)
		delete fragmentUniform3D;
	fragmentUniform3D = new VKUniformBuffer<ThreeDimension::FragmentUniform>(vkInit, quadCount);

	ThreeDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexUniforms3D.push_back(mat);
	vertexUniforms3D.back().color = color;

	ThreeDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragUniforms3D.push_back(tIndex);
}


void VKRenderManager::BeginRender(glm::vec4 bgColor)
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
				for (int i = 0; i < textures.size(); ++i)
				{
					imageInfos[i].sampler = *textures[i]->GetSampler();
					imageInfos[i].imageView = *textures[i]->GetImageView();
					imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

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
				VkWriteDescriptorSet descriptorWrite[2] = {};
				descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[0].dstSet = *currentVertexMaterialDescriptorSet;
				descriptorWrite[0].dstBinding = 0;
				descriptorWrite[0].descriptorCount = 1;
				descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite[0].pBufferInfo = &bufferInfo;

				VkDescriptorBufferInfo lightingBufferInfo;
				lightingBufferInfo.buffer = (*(vertexLightingUniformBuffer->GetUniformBuffers()))[frameIndex];
				lightingBufferInfo.offset = 0;
				lightingBufferInfo.range = sizeof(ThreeDimension::VertexLightingUniform);

				descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[1].dstSet = *currentVertexMaterialDescriptorSet;
				descriptorWrite[1].dstBinding = 1;
				descriptorWrite[1].descriptorCount = 1;
				descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite[1].pBufferInfo = &lightingBufferInfo;

				//Update DescriptorSet
				//DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 2, descriptorWrite, 0, nullptr);
			}
			vertexUniform3D->UpdateUniform(vertexUniforms3D.size(), vertexUniforms3D.data(), frameIndex);
			vertexLightingUniformBuffer->UpdateUniform(1, &vertexLightingUniform, frameIndex);
		}

		if (fragmentUniform3D != nullptr)
		{
			currentTextureDescriptorSet = &(*vkDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
			{
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*(fragmentUniform3D->GetUniformBuffers()))[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(ThreeDimension::FragmentUniform) * quadCount;

				VkWriteDescriptorSet descriptorWrite[2] = {};
				descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[0].dstSet = *currentTextureDescriptorSet;
				descriptorWrite[0].dstBinding = 0;
				descriptorWrite[0].descriptorCount = 1;
				descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite[0].pBufferInfo = &bufferInfo;

				//Create Texture DescriptorBuffer Info
				for (int i = 0; i < textures.size(); ++i)
				{
					imageInfos[i].sampler = *textures[i]->GetSampler();
					imageInfos[i].imageView = *textures[i]->GetImageView();
					imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

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
			fragmentUniform3D->UpdateUniform(fragUniforms3D.size(), fragUniforms3D.data(), frameIndex);
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
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {bgColor.r, bgColor.g, bgColor.b, bgColor.a} };
	clearValues[1].depthStencil = { 1.0f, 0 };

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
	renderpassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderpassBeginInfo.pClearValues = clearValues.data();

	//Begin renderpass
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
	scissor.offset = { 0,0 };
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
			vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
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
				vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
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
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				break;
			case PolygonType::LINE:
				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, vertex3DBuffer->GetVertexBuffer(), &vertexBufferOffset);
				//Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
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
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				break;
			}
		}
		break;
	}

#ifdef _DEBUG
	imguiManager->Begin();
#endif
}

void VKRenderManager::EndRender()
{
	if (!isRecreated)
	{
#ifdef _DEBUG
		imguiManager->End(frameIndex);
#endif

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
