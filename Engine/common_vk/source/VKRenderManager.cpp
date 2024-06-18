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

VKRenderManager::~VKRenderManager()
{
	vkDeviceWaitIdle(*vkInit->GetDevice());

#ifdef _DEBUG
	//delete imGUI
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
	delete texVertex;
	delete texIndex;
	delete uVertex;
	delete uFragment;

	//Destroy Texture
	for (auto t : textures)
		delete t;

	//Destroy Batch ImageInfo
	size_t texSize{ textures.size() };
	for (size_t i = texSize; i < imageInfos.size(); ++i)
		vkDestroySampler(*vkInit->GetDevice(), imageInfos[i].sampler, nullptr);
	//for(auto& i : imageInfos)
	//	vkDestroySampler(*vkInit->GetDevice(), i.sampler, nullptr);

	textures.erase(textures.begin(), textures.end());
	imageInfos.erase(imageInfos.begin(), imageInfos.end());
	
	//Destroy Shader
	delete vkTextureShader;
	delete vkLineShader;

	//Destroy Pipeline
	delete vkTexurePipeline;
	//delete vkLinePipeline;

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

	InitRenderPass();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());

	vkDescriptor = new VKDescriptor(vkInit);

	vkTextureShader = new VKShader(vkInit->GetDevice());
	vkTextureShader->LoadShader("../Engine/shader/texVertex.vert", "../Engine/shader/texFragment.frag");
	vkLineShader = new VKShader(vkInit->GetDevice());
	vkLineShader->LoadShader("../Engine/shader/lineVertex.vert", "../Engine/shader/lineFragment.frag");
	std::cout << std::endl;

	vkTexurePipeline = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkTexurePipeline->InitPipeLine(vkTextureShader->GetVertexModule(), vkTextureShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, POLYGON_MODE::FILL);
	//vkLinePipeline = new VKPipeLine(vkDescriptor->GetDescriptorSetLayout());
	//vkLinePipeline->InitPipeLine(vkLineShader->GetVertexModule(), vkLineShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, POLYGON_MODE::LINE);

#ifdef _DEBUG
	imguiManager = new VKImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor->GetDescriptorPool(), &vkRenderPass);
#endif

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

	//Create Subpass Description
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;

	//Create Renderpass Info
	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &attachmentDescription;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDescription;

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
	//Create framebuffer info
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = vkRenderPass;
	createInfo.attachmentCount = 1;
	createInfo.width = swapchainImageExtent_->width;
	createInfo.height = swapchainImageExtent_->height;
	createInfo.layers = 1;

	//Allocate memory for framebuffers
	vkFrameBuffers.resize(swapchainImageViews_->size());

	try
	{
		for (auto i = 0; i != swapchainImageViews_->size(); ++i)
		{
			createInfo.pAttachments = &(*swapchainImageViews_)[i];

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
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKRenderManager::~VKRenderManager();
		std::exit(EXIT_FAILURE);
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
	texVertices.push_back(VKVertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), quadCount));
	texVertices.push_back(VKVertex(glm::vec4(1.f, 1.f, 1.f, 1.f), quadCount));
	texVertices.push_back(VKVertex(glm::vec4(1.f, -1.f, 1.f, 1.f), quadCount));
	texVertices.push_back(VKVertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), quadCount));
	if (texVertex != nullptr)
		delete texVertex;
	texVertex = new VKVertexBuffer(vkInit, &texVertices);

	uint64_t indexNumber{ texVertices.size() / 4 - 1 };
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 1));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 3));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber));
	if (texIndex != nullptr)
		delete texIndex;
	texIndex = new VKIndexBuffer(vkInit, &vkCommandPool, &texIndices);

	quadCount++;

	if (uVertex != nullptr)
		delete uVertex;
	uVertex = new VKUniformBuffer<VKVertexUniform>(vkInit, quadCount);

	if (uFragment != nullptr)
		delete uFragment;
	uFragment = new VKUniformBuffer<VKFragmentUniform>(vkInit, quadCount);

	VKVertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexVector.push_back(mat);
	vertexVector.back().color = color_;
	vertexVector.back().isTex = isTex_;
	vertexVector.back().isTexel = isTexel_;

	VKFragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragVector.push_back(tIndex);
}

void VKRenderManager::DeleteWithIndex()
{
	quadCount--;

	if (quadCount == 0)
	{
		texVertices.erase(end(texVertices) - 4, end(texVertices));
		delete texVertex;
		texVertex = nullptr;

		texIndices.erase(end(texIndices) - 6, end(texIndices));
		delete texIndex;
		texIndex = nullptr;

		vertexVector.erase(end(vertexVector) - 1);
		delete uVertex;
		uVertex = nullptr;

		fragVector.erase(end(fragVector) - 1);
		delete uFragment;
		uFragment = nullptr;

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

	texVertices.erase(end(texVertices) - 4, end(texVertices));
	vkCmdUpdateBuffer(commandBuffer, *texVertex->GetVertexBuffer(), 0, texVertices.size() * sizeof(VKVertex), texVertices.data());

	texIndices.erase(end(texIndices) - 6, end(texIndices));
	vkCmdUpdateBuffer(commandBuffer, *texIndex->GetIndexBuffer(), 0, texIndices.size() * sizeof(uint16_t), texIndices.data());

	vertexVector.erase(end(vertexVector) - 1);
	for (auto u : *uVertex->GetUniformBuffers())
	{
		vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(VKVertexUniform), vertexVector.data());
	}

	fragVector.erase(end(fragVector) - 1);
	for (auto u : *uFragment->GetUniformBuffers())
	{
		vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(VKFragmentUniform), fragVector.data());
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

void VKRenderManager::BeginRender()
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

	if (uVertex != nullptr)
	{
		currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
		{
			//Create Vertex Material DescriptorBuffer Info
			//std::vector<VkDescriptorBufferInfo> bufferInfos;
			//for (auto& t : textures)
			//{
			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = (*(uVertex->GetUniformBuffers()))[frameIndex];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(VKVertexUniform) * quadCount;
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
		uVertex->UpdateUniform(vertexVector, frameIndex);
	}

	if (uFragment != nullptr)
	{
		currentTextureDescriptorSet = &(*vkDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
		{
			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = (*(uFragment->GetUniformBuffers()))[frameIndex];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(VKFragmentUniform) * quadCount;

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
		uFragment->UpdateUniform(fragVector, frameIndex);
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
	VkClearValue clearValue{};
	clearValue.color.float32[0] = 0.0f;	//R
	clearValue.color.float32[1] = 0.0f;	//G
	clearValue.color.float32[2] = 0.7f;	//B
	clearValue.color.float32[3] = 1.0f;	//A

	//Create renderpass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = vkRenderPass;
	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearValue;

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

	if (texVertex != nullptr)
	{
		//Bind Vertex Buffer
		vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, texVertex->GetVertexBuffer(), &vertexBufferOffset);
		//Bind Index Buffer
		vkCmdBindIndexBuffer(*currentCommandBuffer, *texIndex->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
		//Bind Pipeline
		vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkTexurePipeline->GetPipeLine());
		//Dynamic Viewport & Scissor
		vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
		//Bind Material DescriptorSet
		vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkTexurePipeline->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
		//Bind Texture DescriptorSet
		vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkTexurePipeline->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
		//Change Primitive Topology
		vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		//Draw
		vkCmdDrawIndexed(*currentCommandBuffer, static_cast<uint32_t>(texIndices.size()), 1, 0, 0, 0);
	}

	//if (lineVertex != nullptr)
	//{
	//	//Bind Vertex Buffer
	//	vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, lineVertex->GetVertexBuffer(), &vertexBufferOffset);
	//	//Bind Index Buffer
	//	vkCmdBindIndexBuffer(*currentCommandBuffer, *lineIndex->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
	//	//Bind Pipeline
	//	vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkLinePipeline->GetPipeLine());
	//	//Dynamic Viewport & Scissor
	//	vkCmdSetViewport(*currentCommandBuffer, 0, 1, &viewport);
	//	vkCmdSetScissor(*currentCommandBuffer, 0, 1, &scissor);
	//	//Bind Material DescriptorSet
	//	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkLinePipeline->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
	//	//Change Line Width
	//	vkCmdSetLineWidth(*currentCommandBuffer, 5.0f);
	//	//Change Primitive Topology
	//	vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//	//Draw
	//	vkCmdDrawIndexed(*currentCommandBuffer, lineIndices.size(), 1, 0, 0, 0);
	//}

#ifdef _DEBUG
	imguiManager->Begin();
#endif
}

void VKRenderManager::EndRender()
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
