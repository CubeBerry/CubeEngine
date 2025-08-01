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
#include "VKHelper.hpp"

#include "Engine.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

VKRenderManager::~VKRenderManager()
{
	VKHelper::ThrowIfFailed(vkDeviceWaitIdle(*vkInit->GetDevice()));

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
	// @TODO Fix destroying unique pointers after VKRenderManager destructor so that uniform buffers can be deleted using VkDevice
	uniformBuffer2D.vertexUniformBuffer.reset();
	uniformBuffer2D.fragmentUniformBuffer.reset();
	uniformBuffer3D.vertexUniformBuffer.reset();
	uniformBuffer3D.fragmentUniformBuffer.reset();
	uniformBuffer3D.materialUniformBuffer.reset();
	delete pointLightUniformBuffer;
	delete directionalLightUniformBuffer;

	//Destroy Batch ImageInfo
	vkDestroySampler(*vkInit->GetDevice(), immutableSampler, nullptr);
	//size_t texSize{ textures.size() };
	//for (size_t i = texSize; i < imageInfos.size(); ++i)
	//	vkDestroySampler(*vkInit->GetDevice(), imageInfos[i].sampler, nullptr);

	textures.erase(textures.begin(), textures.end());
	imageInfos.erase(imageInfos.begin(), imageInfos.end());

	delete vkRenderTarget;

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
	delete vkDescriptor2D;
	delete vkDescriptor3D;
#ifdef _DEBUG
	delete vkDescriptor3DNormal;
#endif

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

	// MSAA
	// Depth Buffering
	vkRenderTarget = new VKRenderTarget(vkInit, vkSwapChain);

	InitRenderPass();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());

	// Initialize Descriptor3D
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
	vkDescriptor3D = new VKDescriptor(vkInit, { vertexLayout }, { fragmentLayout[0], fragmentLayout[1], fragmentLayout[2], fragmentLayout[3], fragmentLayout[4], fragmentLayout[5], fragmentLayout[6], fragmentLayout[7] });

	// Initialize Descriptor2D
	vkDescriptor2D = new VKDescriptor(vkInit, { vertexLayout }, { fragmentLayout[0], fragmentLayout[1] });

#ifdef _DEBUG
	// Initialize Descriptor3DNormal
	vkDescriptor3DNormal = new VKDescriptor(vkInit, {}, {});
#endif

	vkShader2D = new VKShader(vkInit->GetDevice());
	vkShader2D->LoadShader("../Engine/shaders/spirv/2D.vert.spv", "../Engine/shaders/spirv/2D.frag.spv");

	vkShader3D = new VKShader(vkInit->GetDevice());
	vkShader3D->LoadShader("../Engine/shaders/spirv/3D.vert.spv", "../Engine/shaders/spirv/3D.frag.spv");

#ifdef _DEBUG
	vkNormal3DShader = new VKShader(vkInit->GetDevice());
	vkNormal3DShader->LoadShader("../Engine/shaders/spirv/Normal3D.vert.spv", "../Engine/shaders/spirv/Normal3D.frag.spv");
#endif

	// 2D Pipeline
	VKAttributeLayout position_layout;
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32_UINT;
	position_layout.offset = offsetof(TwoDimension::Vertex, position);

	vkPipeline2D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor2D->GetDescriptorSetLayout());
	vkPipeline2D->InitPipeLine(vkShader2D->GetVertexModule(), vkShader2D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(TwoDimension::Vertex), { position_layout }, vkRenderTarget->GetMSAASamples(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, false);

	// 3D Pipeline
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32_UINT;
	position_layout.offset = offsetof(ThreeDimension::QuantizedVertex, position);

	VKAttributeLayout normal_layout;
	normal_layout.vertex_layout_location = 1;
	normal_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal_layout.offset = offsetof(ThreeDimension::QuantizedVertex, normal);

	VKAttributeLayout uv_layout;
	uv_layout.vertex_layout_location = 2;
	uv_layout.format = VK_FORMAT_R32G32_SFLOAT;
	uv_layout.offset = offsetof(ThreeDimension::QuantizedVertex, uv);

	VKAttributeLayout tex_sub_index_layout;
	tex_sub_index_layout.vertex_layout_location = 3;
	tex_sub_index_layout.format = VK_FORMAT_R32_SINT;
	tex_sub_index_layout.offset = offsetof(ThreeDimension::QuantizedVertex, texSubIndex);

	vkPipeline3D = new VKPipeLine(vkInit->GetDevice(), vkDescriptor3D->GetDescriptorSetLayout());
	vkPipeline3D->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout }, vkRenderTarget->GetMSAASamples(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL, true, sizeof(PushConstants), VK_SHADER_STAGE_FRAGMENT_BIT);
	vkPipeline3DLine = new VKPipeLine(vkInit->GetDevice(), vkDescriptor3D->GetDescriptorSetLayout());
	vkPipeline3DLine->InitPipeLine(vkShader3D->GetVertexModule(), vkShader3D->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout }, vkRenderTarget->GetMSAASamples(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::LINE, true, sizeof(PushConstants), VK_SHADER_STAGE_FRAGMENT_BIT);
#ifdef _DEBUG
	position_layout.vertex_layout_location = 0;
	position_layout.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_layout.offset = offsetof(ThreeDimension::NormalVertex, position);

	vkPipeline3DNormal = new VKPipeLine(vkInit->GetDevice(), vkDescriptor3DNormal->GetDescriptorSetLayout());
	vkPipeline3DNormal->InitPipeLine(vkNormal3DShader->GetVertexModule(), vkNormal3DShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(ThreeDimension::NormalVertex), { position_layout }, vkRenderTarget->GetMSAASamples(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_CULL_MODE_BACK_BIT, POLYGON_MODE::FILL, true, sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
#endif

	// Uniform
	uniformBuffer2D.vertexUniformBuffer = std::make_unique<VKUniformBuffer<TwoDimension::VertexUniform>>(vkInit, MAX_OBJECT_SIZE);
	uniformBuffer2D.fragmentUniformBuffer = std::make_unique<VKUniformBuffer<TwoDimension::FragmentUniform>>(vkInit, MAX_OBJECT_SIZE);
	uniformBuffer3D.vertexUniformBuffer = std::make_unique<VKUniformBuffer<ThreeDimension::VertexUniform>>(vkInit, MAX_OBJECT_SIZE);
	uniformBuffer3D.fragmentUniformBuffer = std::make_unique<VKUniformBuffer<ThreeDimension::FragmentUniform>>(vkInit, MAX_OBJECT_SIZE);
	uniformBuffer3D.materialUniformBuffer = std::make_unique<VKUniformBuffer<ThreeDimension::Material>>(vkInit, MAX_OBJECT_SIZE);

	pointLightUniformBuffer = new VKUniformBuffer<ThreeDimension::PointLightUniform>(vkInit, MAX_LIGHT_SIZE);
	directionalLightUniformBuffer = new VKUniformBuffer<ThreeDimension::DirectionalLightUniform>(vkInit, MAX_LIGHT_SIZE);

	imguiManager = new VKImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor2D->GetDescriptorPool(), &vkRenderPass, vkRenderTarget->GetMSAASamples());

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	VKHelper::ThrowIfFailed(vkCreateSampler(*vkInit->GetDevice(), &samplerInfo, nullptr, &immutableSampler));
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
	VKHelper::ThrowIfFailed(vkCreateCommandPool(*vkInit->GetDevice(), &commandPoolInfo, nullptr, &vkCommandPool));
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
	VKHelper::ThrowIfFailed(vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &vkCommandBuffers[0]));
}

void VKRenderManager::InitRenderPass()
{
	VkSurfaceFormatKHR surfaceFormat = vkInit->SetSurfaceFormat();

	//Create Attachment Description
	VkAttachmentDescription colorAattachmentDescription{};
	colorAattachmentDescription.format = surfaceFormat.format;
	colorAattachmentDescription.samples = vkRenderTarget->GetMSAASamples();
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
	depthAttachmentDescription.format = vkRenderTarget->GetDepthFormat();
	depthAttachmentDescription.samples = vkRenderTarget->GetMSAASamples();
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
	VKHelper::ThrowIfFailed(vkCreateRenderPass(*vkInit->GetDevice(), &createInfo, nullptr, &vkRenderPass));
}

void VKRenderManager::InitFrameBuffer(VkExtent2D* swapchainImageExtent_, std::vector<VkImageView>* swapchainImageViews_)
{
	//Allocate memory for framebuffers
	vkFrameBuffers.resize(swapchainImageViews_->size());

	for (int i = 0; i < swapchainImageViews_->size(); ++i)
	{
		VkImageView attachments[3] = { vkRenderTarget->GetColorImageView(), vkRenderTarget->GetDepthImageView(), (*swapchainImageViews_)[i] };

		//Create framebuffer info
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = vkRenderPass;
		createInfo.attachmentCount = 3;
		createInfo.pAttachments = &attachments[0];
		createInfo.width = swapchainImageExtent_->width;
		createInfo.height = swapchainImageExtent_->height;
		createInfo.layers = 1;

		//Create framebuffer
		VKHelper::ThrowIfFailed(vkCreateFramebuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkFrameBuffers[i]));
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
	SDL_GetWindowSizeInPixels(window, &width, &height);

	VKHelper::ThrowIfFailed(vkDeviceWaitIdle(*vkInit->GetDevice()));

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
	delete vkRenderTarget;
	vkRenderTarget = new VKRenderTarget(vkInit, vkSwapChain);
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());
}

void VKRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
	const auto& texture = textures.emplace_back(std::make_unique<VKTexture>(vkInit, &vkCommandPool));
	texture->LoadTexture(false, path_, name_, flip);

	const int texId = static_cast<int>(textures.size() - 1);
	texture->SetTextureID(texId);

	imageInfos[texId].sampler = *textures[texId]->GetSampler();
	imageInfos[texId].imageView = *textures[texId]->GetImageView();
	imageInfos[texId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void VKRenderManager::ClearTextures()
{
	//Destroy Batch ImageInfo
	textures.clear();
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
			return tex.get();
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
	vkPipeline3DSkybox->InitPipeLine(skyboxShader->GetVertexModule(), skyboxShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, sizeof(float) * 3, { position_layout }, vkRenderTarget->GetMSAASamples(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, POLYGON_MODE::FILL, true, sizeof(glm::mat4) * 2, VK_SHADER_STAGE_VERTEX_BIT);

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
			currentVertexDescriptorSet = &(*vkDescriptor2D->GetVertexDescriptorSets())[frameIndex];
			{
				//Create Vertex Material DescriptorBuffer Info
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*vertexUniformBuffer->GetUniformBuffers())[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(TwoDimension::VertexUniform);

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
			}

			auto& fragmentUniformBuffer = uniformBuffer2D.fragmentUniformBuffer;
			currentFragmentDescriptorSet = &(*vkDescriptor2D->GetFragmentDescriptorSets())[frameIndex];
			{
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*fragmentUniformBuffer->GetUniformBuffers())[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(TwoDimension::FragmentUniform);

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
			}
		}
		break;
	case RenderType::ThreeDimension:
		{
			auto& vertexUniformBuffer = uniformBuffer3D.vertexUniformBuffer;
			currentVertexDescriptorSet = &(*vkDescriptor3D->GetVertexDescriptorSets())[frameIndex];
			{
				// Create Vertex Material DescriptorBuffer Info
				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = (*vertexUniformBuffer->GetUniformBuffers())[frameIndex];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(ThreeDimension::VertexUniform);

				// Define which resource descriptor set will point
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *currentVertexDescriptorSet;
				descriptorWrite.dstBinding = 0;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorWrite.pBufferInfo = &bufferInfo;

				// Update DescriptorSet
				// DescriptorSet does not have to update every frame since it points same uniform buffer
				vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}

			auto& fragmentUniformBuffer = uniformBuffer3D.fragmentUniformBuffer;
			auto& materialUniformBuffer = uniformBuffer3D.materialUniformBuffer;
			currentFragmentDescriptorSet = &(*vkDescriptor3D->GetFragmentDescriptorSets())[frameIndex];
			{
				std::vector<VkWriteDescriptorSet> descriptorWrites;

				VkDescriptorBufferInfo fragmentBufferInfo{};
				fragmentBufferInfo.buffer = (*fragmentUniformBuffer->GetUniformBuffers())[frameIndex];
				fragmentBufferInfo.offset = 0;
				fragmentBufferInfo.range = sizeof(ThreeDimension::FragmentUniform);

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
				materialBufferInfo.range = sizeof(ThreeDimension::Material);

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
					pointLightDescriptorWrite.dstSet = *currentFragmentDescriptorSet;
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

	// Reset command buffer
	vkResetCommandBuffer(*currentCommandBuffer, 0);

	// Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Begin command buffer
	VKHelper::ThrowIfFailed(vkBeginCommandBuffer(*currentCommandBuffer, &beginInfo));

	// Change image layout to TRANSFER_DST_OPTIMAL
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

		// Record pipeline barrier for chainging image layout
		vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	// Set clear color
	VkClearValue clearValues[2];
	clearValues[0].color = { {bgColor.r, bgColor.g, bgColor.b, 1.f} };
	clearValues[1].depthStencil = { 1.f, 0 };

	// Create RenderPass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = vkRenderPass;
	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearValues;

	// Begin RenderPass
	vkCmdBeginRenderPass(*currentCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//--------------------Begin Draw--------------------//

	// Create Viewport and Scissor for Dynamic State
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

	VkDeviceSize vertexBufferOffset{ 0 };

	switch (rMode)
	{
	case RenderType::TwoDimension:
	{
		void* vertexMappedMemory = uniformBuffer2D.vertexUniformBuffer->GetMappedMemory(frameIndex);
		void* fragmentMappedMemory = uniformBuffer2D.fragmentUniformBuffer->GetMappedMemory(frameIndex);

		// Bind Pipeline
		vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLine());
		for (size_t i = 0; i < sprites.size(); ++i)
		{
			for (auto& subMesh : sprites[i]->GetSubMeshes())
			{
				auto& buffer = subMesh.bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>();
				// Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, buffer.vertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
				// Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *buffer.indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				// Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				// Bind Vertex DescriptorSet
				size_t alignment = vkInit->GetMinUniformBufferOffsetAlignment();
				size_t uniformSize = sizeof(TwoDimension::VertexUniform);
				uint32_t dynamicOffset = static_cast<uint32_t>(i * ((uniformSize + alignment - 1) & ~(alignment - 1)));

				TwoDimension::VertexUniform* vertexDest = (TwoDimension::VertexUniform*)((uint8_t*)vertexMappedMemory + dynamicOffset);
				*vertexDest = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform;
				// @TODO do not use magic number for dynamicOffsetCount
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLineLayout(), 0, 1, currentVertexDescriptorSet, 1, &dynamicOffset);

				// Bind Fragment DescriptorSet
				uniformSize = sizeof(TwoDimension::FragmentUniform);
				dynamicOffset = static_cast<uint32_t>(i * ((uniformSize + alignment - 1) & ~(alignment - 1)));

				TwoDimension::FragmentUniform* fragmentDest = (TwoDimension::FragmentUniform*)((uint8_t*)fragmentMappedMemory + dynamicOffset);
				*fragmentDest = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform;

				// @TODO do not use magic number for dynamicOffsetCount
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline2D->GetPipeLineLayout(), 1, 1, currentFragmentDescriptorSet, 1, &dynamicOffset);
				// Change Primitive Topology
				//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				//Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(subMesh.bufferWrapper.GetIndices().size()), 1, 0, 0, 0);
			}
		}
		uniformBuffer2D.vertexUniformBuffer->UnmapMemory(frameIndex);
		uniformBuffer2D.fragmentUniformBuffer->UnmapMemory(frameIndex);
	}
	break;
	case RenderType::ThreeDimension:
		void* vertexMappedMemory = uniformBuffer3D.vertexUniformBuffer->GetMappedMemory(frameIndex);
		void* fragmentMappedMemory = uniformBuffer3D.fragmentUniformBuffer->GetMappedMemory(frameIndex);
		void* materialMappedMemory = uniformBuffer3D.materialUniformBuffer->GetMappedMemory(frameIndex);

		// @TODO Can I use dynamic polygon type (FILL or LINE)?
		uint64_t subMeshIndex{ 0 };
		for (const auto& sprite : sprites)
		{
			for (auto& subMesh : sprite->GetSubMeshes())
			{
				auto& buffer = subMesh.bufferWrapper.GetBuffer<BufferWrapper::VKBuffer>();

				// Bind Pipeline
				auto* pipeline = pMode == PolygonType::FILL ? vkPipeline3D : vkPipeline3DLine;
				vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->GetPipeLine());
				// Bind Vertex Buffer
				vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, buffer.vertexBuffer->GetVertexBuffer(), &vertexBufferOffset);
				// Bind Index Buffer
				vkCmdBindIndexBuffer(*currentCommandBuffer, *buffer.indexBuffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				// Dynamic Viewport & Scissor
				vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
				// Bind Vertex DescriptorSet
				size_t alignment = vkInit->GetMinUniformBufferOffsetAlignment();
				size_t uniformSize = sizeof(ThreeDimension::VertexUniform);
				uint32_t dynamicOffset = static_cast<uint32_t>(subMeshIndex * ((uniformSize + alignment - 1) & ~(alignment - 1)));

				ThreeDimension::VertexUniform* vertexDest = (ThreeDimension::VertexUniform*)((uint8_t*)vertexMappedMemory + dynamicOffset);
				*vertexDest = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
				// @TODO do not use magic number for dynamicOffsetCount
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->GetPipeLineLayout(), 0, 1, currentVertexDescriptorSet, 1, &dynamicOffset);

				// Bind Fragment DescriptorSet
				uint32_t dynamicOffsets[2];
				// Fragment Uniform Offset
				uniformSize = sizeof(ThreeDimension::FragmentUniform);
				dynamicOffset = static_cast<uint32_t>(subMeshIndex * ((uniformSize + alignment - 1) & ~(alignment - 1)));
				dynamicOffsets[0] = dynamicOffset;

				ThreeDimension::FragmentUniform* fragmentDest = (ThreeDimension::FragmentUniform*)((uint8_t*)fragmentMappedMemory + dynamicOffset);
				*fragmentDest = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;

				// Material Uniform Offset
				uniformSize = sizeof(ThreeDimension::Material);
				dynamicOffset = static_cast<uint32_t>(subMeshIndex * ((uniformSize + alignment - 1) & ~(alignment - 1)));
				dynamicOffsets[1] = dynamicOffset;

				ThreeDimension::Material* materialDest = (ThreeDimension::Material*)((uint8_t*)materialMappedMemory + dynamicOffset);
				*materialDest = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().material;

				// @TODO do not use magic number for dynamicOffsetCount
				vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->GetPipeLineLayout(), 1, 1, currentFragmentDescriptorSet, 2, dynamicOffsets);

				// Change Primitive Topology
				// vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
				// vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
				// Push Constant Active Lights
				pushConstants.activeDirectionalLight = static_cast<int>(directionalLightUniforms.size());
				pushConstants.activePointLight = static_cast<int>(pointLightUniforms.size());
				vkCmdPushConstants(*currentCommandBuffer, *pipeline->GetPipeLineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
				// Draw
				vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(subMesh.bufferWrapper.GetIndices().size()), 1, 0, 0, 0);

#ifdef _DEBUG
				if (isDrawNormals)
				{
					VkBuffer* normalVertexBuffer = buffer.normalVertexBuffer->GetVertexBuffer();
					//Bind Pipeline
					vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline3DNormal->GetPipeLine());
					//Bind Vertex Buffer
					vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, normalVertexBuffer, &vertexBufferOffset);
					//Dynamic Viewport & Scissor
					vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
					vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
					//Change Primitive Topology
					//vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
					//Push Constant Model-To_NDC
					auto& vertexUniform = subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
					glm::mat4 modelToNDC = vertexUniform.projection * vertexUniform.view * vertexUniform.model;
					vkCmdPushConstants(*currentCommandBuffer, *vkPipeline3DNormal->GetPipeLineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelToNDC);
					//Draw
					vkCmdDraw(*currentCommandBuffer, static_cast<uint32_t>(subMesh.bufferWrapper.GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices.size()), 1, 0, 0);
				}
#endif

				subMeshIndex++;
			}
		}

		uniformBuffer3D.vertexUniformBuffer->UnmapMemory(frameIndex);
		uniformBuffer3D.fragmentUniformBuffer->UnmapMemory(frameIndex);
		uniformBuffer3D.materialUniformBuffer->UnmapMemory(frameIndex);

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
			//Bind Vertex DescriptorSet
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
		VKHelper::ThrowIfFailed(vkEndCommandBuffer(*currentCommandBuffer));

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
		VKHelper::ThrowIfFailed(vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *currentFence));

		//Wait until all submitted command buffers are handled
		VKHelper::ThrowIfFailed(vkDeviceWaitIdle(*vkInit->GetDevice()));

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
