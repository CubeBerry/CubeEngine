#include "VKRenderManager.hpp"
//#include "VKInit.hpp"
//#include "VKSwapChain.hpp"
#include "VKShader.hpp"
#include "VKPipeLine.hpp"
#include "Window.hpp"
#include "VKUniformBuffer.hpp"
#include "Engine.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

VKRenderManager::VKRenderManager(SDL_Window* window_, bool isDiscrete) : window(window_)
{
	vkInit = new VKInit(window, isDiscrete);
	vkSwapChain = new VKSwapChain(vkInit);
	
	InitCommandPool();
	InitCommandBuffer();

	vkSwapChain->InitSwapChainImage(&vkCommandBuffers[0]);
	vkSwapChain->InitFence();
	vkSwapChain->InitSemaphore();
	vkSwapChain->InitSwapChainImageView();

	InitRenderPass();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());

	vkDescriptor = new VKDescriptor(vkInit);

	vkTextureShader = new VKShader(vkInit->GetDevice());
	vkTextureShader->LoadShader("../Engine/shader/texVertex.vert", "../Engine/shader/texFragment.frag");
	vkLineShader = new VKShader(vkInit->GetDevice());
	vkLineShader->LoadShader("../Engine/shader/lineVertex.vert", "../Engine/shader/lineFragment.frag");

	vkTexurePipeline = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkTexurePipeline->InitPipeLine(vkTextureShader->GetVertexModule(), vkTextureShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, POLYGON_MODE::FILL);
	vkLinePipeline = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkLinePipeline->InitPipeLine(vkLineShader->GetVertexModule(), vkLineShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass, POLYGON_MODE::LINE);

	imguiManager = new ImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor->GetDescriptorPool(), &vkRenderPass);

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

VKRenderManager::~VKRenderManager()
{
	//Destroy Command Pool, also Command Buffer destroys with Command Pool
	vkDestroyCommandPool(*vkInit->GetDevice(), vkCommandPool, nullptr);
	//Destroy RenderPass
	vkDestroyRenderPass(*vkInit->GetDevice(), vkRenderPass, nullptr);
	//Destroy FrameBuffer
	for (auto& framebuffer : vkFrameBuffers)
	{
		vkDestroyFramebuffer(*vkInit->GetDevice(), framebuffer, nullptr);
	}

	delete vkSwapChain;
	delete vkInit;
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

void VKRenderManager::RecreateSwapChain(Window* window_)
{
	int width = 0, height = 0;
	while (window_->GetMinimized())
	{
		SDL_PumpEvents();
	}
	SDL_GL_GetDrawableSize(window, &width, &height);

	vkDeviceWaitIdle(*vkInit->GetDevice());

	CleanSwapChain();

	vkSwapChain->InitSwapChain();
	vkSwapChain->InitSwapChainImage(&vkCommandBuffers[0]);
	vkSwapChain->InitSwapChainImageView();
	InitFrameBuffer(vkSwapChain->GetSwapChainImageExtent(), vkSwapChain->GetSwapChainImageViews());
}

//void VKRenderManager::OldClearColor()
//{
//	//Get image index from swapchain
//	uint32_t swapchainIndex_;
//	vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, (*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex_);
//
//	//Get swapchain image
//	VkImage swapchainImage_ = (*vkSwapChain->GetSwapChainImages())[swapchainIndex_];
//
//	//Wait for fence to be signaled
//	if (vkGetFenceStatus(*vkInit->GetDevice(), *vkSwapChain->GetFence()) == VK_NOT_READY)
//		vkWaitForFences(*vkInit->GetDevice(), 1, vkSwapChain->GetFence(), VK_TRUE, UINT64_MAX);
//
//	//Set fence to unsignaled
//	vkResetFences(*vkInit->GetDevice(), 1, vkSwapChain->GetFence());
//
//	//Reset command buffer
//	vkResetCommandBuffer(vkCommandBuffer, 0);
//
//	//Create command buffer begin info
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	//Begin command buffer
//	vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
//
//	//Change image layout to TRANSFER_DST_OPTIMAL
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = 0;
//		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//Set clear color
//	VkClearColorValue clearColor{};
//	clearColor.float32[0] = 1.0f;	//R
//	clearColor.float32[1] = 0.0f;	//G
//	clearColor.float32[2] = 1.0f;	//B
//	clearColor.float32[3] = 1.0f;	//A
//
//	//Set subresource range
//	VkImageSubresourceRange subresourceRange{};
//	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	subresourceRange.levelCount = 1;
//	subresourceRange.layerCount = 1;
//
//	//Clear color
//	vkCmdClearColorImage(vkCommandBuffer, swapchainImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);
//
//	//Change image layout to PRESENT_SRC_KHR
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//		barrier.dstAccessMask = 0;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage_;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//End command buffer
//	vkEndCommandBuffer(vkCommandBuffer);
//
//	//Create submit info
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	//Wait until swapchain image is ready after calculating pixel's result
//	//Define pipeline stage that semaphore must be signaled
//	constexpr VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX];
//	submitInfo.pWaitDstStageMask = &waitDstStageMask;
//
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &vkCommandBuffer;
//
//	//Define semaphore that informs when command buffer is processed
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	//Submit queue to command buffer
//	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *vkSwapChain->GetFence());
//
//	//Wait until all submitted command buffers are handled
//	vkDeviceWaitIdle(*vkInit->GetDevice());
//
//	//Create present info
//	VkPresentInfoKHR presentInfo{};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//	//Define semaphore that waits to ensure command buffer to be processed
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = vkSwapChain->GetSwapChain();
//	presentInfo.pImageIndices = &swapchainIndex;
//
//	//Render image on screen
//	vkQueuePresentKHR(*vkInit->GetQueue(), &presentInfo);
//}

//void VKRenderManager::NewClearColor(Window* window_)
//{
//	//Get image index from swapchain
//	//uint32_t swapchainIndex;
//	VkResult result = vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, (*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex);
//	if (result == VK_ERROR_OUT_OF_DATE_KHR)
//		RecreateSwapChain(window_);
//	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	//	throw std::runtime_error("Failed Acquring SwapChain Image");
//
//	//Get swapchain image
//	//VkImage swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
//	swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
//
//	//Wait for fence to be signaled
//	if (vkGetFenceStatus(*vkInit->GetDevice(), *vkSwapChain->GetFence()) == VK_NOT_READY)
//		vkWaitForFences(*vkInit->GetDevice(), 1, vkSwapChain->GetFence(), VK_TRUE, UINT64_MAX);
//
//	//Set fence to unsignaled
//	vkResetFences(*vkInit->GetDevice(), 1, vkSwapChain->GetFence());
//
//	//Reset command buffer
//	vkResetCommandBuffer(vkCommandBuffer, 0);
//
//	//Create command buffer begin info
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	//Begin command buffer
//	vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
//
//	//Change image layout to TRANSFER_DST_OPTIMAL
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = 0;
//		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//Set clear color
//	VkClearValue clearValue{};
//	clearValue.color.float32[0] = 1.0f;	//R
//	clearValue.color.float32[1] = 1.0f;	//G
//	clearValue.color.float32[2] = 0.0f;	//B
//	clearValue.color.float32[3] = 1.0f;	//A
//
//	//Create renderpass begin info
//	VkRenderPassBeginInfo renderpassBeginInfo{};
//	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderpassBeginInfo.renderPass = vkRenderPass;
//	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
//	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
//	renderpassBeginInfo.clearValueCount = 1;
//	renderpassBeginInfo.pClearValues = &clearValue;
//
//	//Begin renderpass
//	vkCmdBeginRenderPass(vkCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	//End renderpass
//	vkCmdEndRenderPass(vkCommandBuffer);
//
//	//Change image layout to PRESENT_SRC_KHR
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		barrier.dstAccessMask = 0;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//End command buffer
//	vkEndCommandBuffer(vkCommandBuffer);
//
//	//Create submit info
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	//Wait until swapchain image is ready after calculating pixel's result
//	//Define pipeline stage that semaphore must be signaled
//	constexpr VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX];
//	submitInfo.pWaitDstStageMask = &waitDstStageMask;
//
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &vkCommandBuffer;
//
//	//Define semaphore that informs when command buffer is processed
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	//Submit queue to command buffer
//	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *vkSwapChain->GetFence());
//
//	//Wait until all submitted command buffers are handled
//	vkDeviceWaitIdle(*vkInit->GetDevice());
//
//	//Create present info
//	VkPresentInfoKHR presentInfo{};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//	//Define semaphore that waits to ensure command buffer to be processed
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = vkSwapChain->GetSwapChain();
//	presentInfo.pImageIndices = &swapchainIndex;
//
//	//Render image on screen
//	VkResult result2 = vkQueuePresentKHR(*vkInit->GetQueue(), &presentInfo);
//	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR)
//		RecreateSwapChain(window_);
//	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	//	throw std::runtime_error("Failed Acquring SwapChain Image");
//}

//void VKRenderManager::DrawVerticesTriangle(VkBuffer* buffer_)
//{
//	//Draw Triangle
//	VkDeviceSize vertexBufferOffset{ 0 };
//	vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, buffer_, &vertexBufferOffset);
//
//	vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLine());
//	vkCmdDraw(*currentCommandBuffer, 3, 1, 0, 0);
//}

//void VKRenderManager::DrawIndicesTriangle(VkBuffer* vertex_, VkBuffer* index_)
//{
//	//Draw Triangle
//	VkDeviceSize vertexBufferOffset{ 0 };
//	//Bind Vertex Buffer
//	vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, vertex_, &vertexBufferOffset);
//	//Bind Index Buffer
//	vkCmdBindIndexBuffer(*currentCommandBuffer, *index_, 0, VK_INDEX_TYPE_UINT16);
//	//Bind Pipeline
//	vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLine());
//	//Bind Material DescriptorSet
//	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLineLayout(), 0, 1, currentMaterialDescriptorSet, 0, nullptr);
//	//Bind Texture DescriptorSet
//	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
//	//Draw
//	vkCmdDrawIndexed(*currentCommandBuffer, 3, 1, 0, 0, 0);
//}

//template<typename Material>
//void VKRenderManager::BeginRender(Window* window_, VKUniformBuffer<Material>* uniform_, Material* material_)
//{
//	//Get image index from swapchain
//	//uint32_t swapchainIndex;
//	VkResult result = vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, (*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex);
//	if (result == VK_ERROR_OUT_OF_DATE_KHR)
//		RecreateSwapChain(window_);
//	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	//	throw std::runtime_error("Failed Acquring SwapChain Image");
//
//	//Get swapchain image
//	//VkImage swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
//	swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
//
//	//Wait for fence to be signaled
//	if (vkGetFenceStatus(*vkInit->GetDevice(), *vkSwapChain->GetFence()) == VK_NOT_READY)
//		vkWaitForFences(*vkInit->GetDevice(), 1, vkSwapChain->GetFence(), VK_TRUE, UINT64_MAX);
//
//	//Set fence to unsignaled
//	vkResetFences(*vkInit->GetDevice(), 1, vkSwapChain->GetFence());
//
//	//Update Uniform Material
//	uniform_->UpdateUniform(material_);
//
//	//Reset command buffer
//	vkResetCommandBuffer(vkCommandBuffer, 0);
//
//	//Create command buffer begin info
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	//Begin command buffer
//	vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
//
//	//Change image layout to TRANSFER_DST_OPTIMAL
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = 0;
//		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//Set clear color
//	VkClearValue clearValue{};
//	clearValue.color.float32[0] = 1.0f;	//R
//	clearValue.color.float32[1] = 0.0f;	//G
//	clearValue.color.float32[2] = 1.0f;	//B
//	clearValue.color.float32[3] = 1.0f;	//A
//
//	//Create renderpass begin info
//	VkRenderPassBeginInfo renderpassBeginInfo{};
//	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderpassBeginInfo.renderPass = vkRenderPass;
//	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
//	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
//	renderpassBeginInfo.clearValueCount = 1;
//	renderpassBeginInfo.pClearValues = &clearValue;
//
//	//Begin renderpass
//	vkCmdBeginRenderPass(vkCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//}

//void VKRenderManager::EndRender(Window* window_)
//{
//	//End renderpass
//	vkCmdEndRenderPass(*currentCommandBuffer);
//
//	//Change image layout to PRESENT_SRC_KHR
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		barrier.dstAccessMask = 0;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//End command buffer
//	vkEndCommandBuffer(*currentCommandBuffer);
//
//	//Create submit info
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	//Wait until swapchain image is ready after calculating pixel's result
//	//Define pipeline stage that semaphore must be signaled
//	constexpr VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX];
//	submitInfo.pWaitDstStageMask = &waitDstStageMask;
//
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = currentCommandBuffer;
//
//	//Define semaphore that informs when command buffer is processed
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	//Submit queue to command buffer
//	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *currentFence);
//
//	//Wait until all submitted command buffers are handled
//	vkDeviceWaitIdle(*vkInit->GetDevice());
//
//	//Create present info
//	VkPresentInfoKHR presentInfo{};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//	//Define semaphore that waits to ensure command buffer to be processed
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = vkSwapChain->GetSwapChain();
//	presentInfo.pImageIndices = &swapchainIndex;
//
//	//Render image on screen
//	VkResult result2 = vkQueuePresentKHR(*vkInit->GetQueue(), &presentInfo);
//	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR)
//		RecreateSwapChain(window_);
//	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	//	throw std::runtime_error("Failed Acquring SwapChain Image");
//
//	frameIndex = ++frameIndex % *vkSwapChain->GetBufferCount();
//}

void VKRenderManager::LoadTexture(const std::filesystem::path& path_)
{
	//int indexCount{ static_cast<int>(textures.size()) };
	VKTexture* texture = new VKTexture(vkInit, &vkCommandPool);
	texture->LoadTexture(path_);
	//texVertices.push_back(Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, textures.size()));
	//texVertices.push_back(Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, textures.size()));
	//texVertices.push_back(Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, textures.size()));
	//texVertices.push_back(Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, textures.size()));
	//if (textures.size() > 0)
	//	delete texVertex;
	//texVertex = new VKVertexBuffer(vkInit, &texVertices);

	//texIndices.push_back(4 * indexCount);
	//texIndices.push_back(4 * indexCount + 1);
	//texIndices.push_back(4 * indexCount + 2);
	//texIndices.push_back(4 * indexCount + 2);
	//texIndices.push_back(4 * indexCount + 1);
	//texIndices.push_back(4 * indexCount + 3);
	//if (textures.size() > 0)
	//	delete texIndex;
	//texIndex = new VKIndexBuffer(vkInit, &vkCommandPool, &texIndices);

	textures.push_back(texture);
	//quadCount++;

	//if (textures.size() > 1)
	//	delete uniform;
	//uniform = new VKUniformBuffer<UniformMatrix>(vkInit, quadCount);

	//auto& vkUniformBuffer = (*textures[0].GetUniformBuffers())[frameIndex];
	//auto& vkUniformBuffer2 = (*textures[1].GetUniformBuffers())[frameIndex];
	for (int frameIndex = 0; frameIndex != 2; ++frameIndex)
	{
		//currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
		//{
		//	//Create Vertex Material DescriptorBuffer Info
		//	//std::vector<VkDescriptorBufferInfo> bufferInfos;
		//	//for (auto& t : textures)
		//	//{
		//		VkDescriptorBufferInfo bufferInfo;
		//		bufferInfo.buffer = (*(uniform->GetUniformBuffers()))[frameIndex];
		//		bufferInfo.offset = 0;
		//		bufferInfo.range = sizeof(UniformMatrix) * quadCount;
		//		//bufferInfos.push_back(bufferInfo);
		//	//}

		//	//Define which resource descriptor set will point
		//	VkWriteDescriptorSet descriptorWrite{};
		//	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//	descriptorWrite.dstSet = *currentVertexMaterialDescriptorSet;
		//	descriptorWrite.dstBinding = 0;
		//	descriptorWrite.descriptorCount = 1;
		//	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//	descriptorWrite.pBufferInfo = &bufferInfo;

		//	//Update DescriptorSet
		//	//DescriptorSet does not have to update every frame since it points same uniform buffer
		//	vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
		//}

		currentTextureDescriptorSet = &(*vkDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
		{
			//Create Texture DescriptorBuffer Info
			//std::vector<VkDescriptorImageInfo> imageInfos;
			//for (auto& t : textures)
			//{
			//	VkDescriptorImageInfo imageInfo{};
			//	imageInfo.sampler = *t->GetSampler();
			//	imageInfo.imageView = *t->GetImageView();
			//	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			//	imageInfos.push_back(imageInfo);
			//}

			for (int i = 0; i < textures.size(); ++i)
			{
				imageInfos[i].sampler = *textures[i]->GetSampler();
				imageInfos[i].imageView = *textures[i]->GetImageView();
				imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			//Define which resource descriptor set will point
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = *currentTextureDescriptorSet;
			descriptorWrite.dstBinding = 1;
			descriptorWrite.descriptorCount = 500;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.pImageInfo = imageInfos.data();

			//Update DescriptorSet
			//DescriptorSet does not have to update every frame since it points same uniform buffer
			vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	//UniformMatrix mat;
	//mat.model = glm::mat3(1.f);
	//mat.view = glm::mat3(1.f);
	//mat.projection = glm::mat3(1.f);
	//matrices.push_back(mat);
}

void VKRenderManager::LoadQuad(glm::vec4 color_, float isTex_)
{
	texVertices.push_back(Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, isTex_));
	texVertices.push_back(Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, isTex_));
	texVertices.push_back(Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, isTex_));
	texVertices.push_back(Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, isTex_));
	if (texVertex != nullptr)
		delete texVertex;
	texVertex = new VKVertexBuffer(vkInit, &texVertices);

	uint64_t indexNumber{ texVertices.size() / 4 - 1 };
	texIndices.push_back(4 * indexNumber);
	texIndices.push_back(4 * indexNumber + 1);
	texIndices.push_back(4 * indexNumber + 2);
	texIndices.push_back(4 * indexNumber + 2);
	texIndices.push_back(4 * indexNumber + 1);
	texIndices.push_back(4 * indexNumber + 3);
	if (texIndex != nullptr)
		delete texIndex;
	texIndex = new VKIndexBuffer(vkInit, &vkCommandPool, &texIndices);

	quadCount++;

	if (uniform != nullptr)
		delete uniform;
	uniform = new VKUniformBuffer<UniformMatrix>(vkInit, quadCount);

	//auto& vkUniformBuffer = (*textures[0].GetUniformBuffers())[frameIndex];
	//auto& vkUniformBuffer2 = (*textures[1].GetUniformBuffers())[frameIndex];
	for (int frameIndex = 0; frameIndex != 2; ++frameIndex)
	{
		currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
		{
			//Create Vertex Material DescriptorBuffer Info
			//std::vector<VkDescriptorBufferInfo> bufferInfos;
			//for (auto& t : textures)
			//{
			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = (*(uniform->GetUniformBuffers()))[frameIndex];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformMatrix) * quadCount;
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
	}

	UniformMatrix mat;
	mat.model = glm::mat3(1.f);
	mat.view = glm::mat3(1.f);
	mat.projection = glm::mat3(1.f);
	matrices.push_back(mat);
}

void VKRenderManager::LoadLineQuad(glm::vec4 color_)
{
	lineVertices.push_back(Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, 0.f));
	lineVertices.push_back(Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, 0.f));
	lineVertices.push_back(Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, 0.f));
	lineVertices.push_back(Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), quadCount, 0.f));
	if (lineVertex != nullptr)
		delete lineVertex;
	lineVertex = new VKVertexBuffer(vkInit, &lineVertices);

	uint64_t indexNumber{ lineVertices.size() / 4 - 1 };
	lineIndices.push_back(4 * indexNumber);
	lineIndices.push_back(4 * indexNumber + 1);
	lineIndices.push_back(4 * indexNumber + 2);
	lineIndices.push_back(4 * indexNumber + 2);
	lineIndices.push_back(4 * indexNumber + 1);
	lineIndices.push_back(4 * indexNumber + 3);
	if (lineIndex != nullptr)
		delete lineIndex;
	lineIndex = new VKIndexBuffer(vkInit, &vkCommandPool, &lineIndices);

	quadCount++;

	if (uniform != nullptr)
		delete uniform;
	uniform = new VKUniformBuffer<UniformMatrix>(vkInit, quadCount);

	//auto& vkUniformBuffer = (*textures[0].GetUniformBuffers())[frameIndex];
	//auto& vkUniformBuffer2 = (*textures[1].GetUniformBuffers())[frameIndex];
	for (int frameIndex = 0; frameIndex != 2; ++frameIndex)
	{
		currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
		{
			//Create Vertex Material DescriptorBuffer Info
			//std::vector<VkDescriptorBufferInfo> bufferInfos;
			//for (auto& t : textures)
			//{
			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = (*(uniform->GetUniformBuffers()))[frameIndex];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformMatrix) * quadCount;
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
	}

	UniformMatrix mat;
	mat.model = glm::mat3(1.f);
	mat.view = glm::mat3(1.f);
	mat.projection = glm::mat3(1.f);
	matrices.push_back(mat);
}

void VKRenderManager::Render()
{
	Window* window_ = Engine::Engine().GetWindow();
	auto& vkSemaphore = (*vkSwapChain->GetSemaphores())[frameIndex];

	//Get image index from swapchain
	//uint32_t swapchainIndex;
	VkResult result = vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, (*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		RecreateSwapChain(window_);
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

	//auto& vkUniformBuffer = (*uniform_->GetUniformBuffers())[frameIndex];
	//currentMaterialDescriptorSet = &(*vkDescriptor->GetMaterialDescriptorSets())[frameIndex];
	//{
	//	//Create Fragment Material DescriptorBuffer Info
	//	VkDescriptorBufferInfo bufferInfo{};
	//	bufferInfo.buffer = vkUniformBuffer;
	//	bufferInfo.offset = 0;
	//	bufferInfo.range = sizeof(Material);

	//	//Define which resource descriptor set will point
	//	VkWriteDescriptorSet descriptorWrite{};
	//	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	descriptorWrite.dstSet = *currentMaterialDescriptorSet;
	//	descriptorWrite.dstBinding = 0;
	//	descriptorWrite.descriptorCount = 1;
	//	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//	descriptorWrite.pBufferInfo = &bufferInfo;

	//	//Update DescriptorSet
	//	//DescriptorSet does not have to update every frame since it points same uniform buffer
	//	vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	//}

	//Update Uniform Material

	//Includes Updating Uniform Function
	//textures[0].Resize(mats.data(), frameIndex);

	VkDeviceMemory vkUniformDeviceMemory = (*(uniform->GetUniformDeviceMemories()))[frameIndex];

	//Get Virtual Address for CPU to access Memory
	void* contents;
	vkMapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory, 0, sizeof(UniformMatrix) * quadCount, 0, &contents);

	//auto material = static_cast<UniformMatrix*>(contents);
	//*material = *mats.data();
	memcpy(contents, matrices.data(), sizeof(UniformMatrix) * quadCount);

	vkUnmapMemory(*vkInit->GetDevice(), vkUniformDeviceMemory);

	//textures[1].Resize(uniMat2, frameIndex);
	//textures[2].Resize(uniMat3, frameIndex);
	//uniform_->UpdateUniform(mat, frameIndex);

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
	clearValue.color.float32[0] = 1.0f;	//R
	clearValue.color.float32[1] = 0.0f;	//G
	clearValue.color.float32[2] = 1.0f;	//B
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
	
	//Draw Quad
	VkDeviceSize vertexBufferOffset{ 0 };
	//Bind Vertex Buffer
	vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, texVertex->GetVertexBuffer(), &vertexBufferOffset);
	//Bind Index Buffer
	vkCmdBindIndexBuffer(*currentCommandBuffer, *texIndex->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
	//Bind Pipeline
	vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkTexurePipeline->GetPipeLine());
	//Bind Material DescriptorSet
	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkTexurePipeline->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
	//Bind Texture DescriptorSet
	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkTexurePipeline->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
	//Change Primitive Topology
	vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//Draw
	vkCmdDrawIndexed(*currentCommandBuffer, texIndices.size(), 1, 0, 0, 0);

	//Bind Vertex Buffer
	vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, lineVertex->GetVertexBuffer(), &vertexBufferOffset);
	//Bind Index Buffer
	vkCmdBindIndexBuffer(*currentCommandBuffer, *lineIndex->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
	//Bind Pipeline
	vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkLinePipeline->GetPipeLine());
	//Bind Material DescriptorSet
	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkLinePipeline->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
	//Change Line Width
	vkCmdSetLineWidth(*currentCommandBuffer, 5.0f);
	//Change Primitive Topology
	vkCmdSetPrimitiveTopology(*currentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//Draw
	vkCmdDrawIndexed(*currentCommandBuffer, lineIndices.size(), 1, 0, 0, 0);

	//ImGui
	imguiManager->Begin();
	ImGui::ShowDemoWindow();
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
	submitInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX];
	submitInfo.pWaitDstStageMask = &waitDstStageMask;

	submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = currentCommandBuffer;
	submitInfo.pCommandBuffers = &(*imguiManager->GetCommandBuffers())[frameIndex];

	//Define semaphore that informs when command buffer is processed
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];

	//Submit queue to command buffer
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *currentFence);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());

	//Create present info
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	//Define semaphore that waits to ensure command buffer to be processed
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = vkSwapChain->GetSwapChain();
	presentInfo.pImageIndices = &swapchainIndex;

	//Render image on screen
	VkResult result2 = vkQueuePresentKHR(*vkInit->GetQueue(), &presentInfo);
	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR)
		RecreateSwapChain(window_);
	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	//	throw std::runtime_error("Failed Acquring SwapChain Image");

	frameIndex = ++frameIndex % *vkSwapChain->GetBufferCount();
}