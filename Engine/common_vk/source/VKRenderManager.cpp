#include "VKRenderManager.hpp"
//#include "VKInit.hpp"
//#include "VKSwapChain.hpp"
#include "VKShader.hpp"
#include "VKPipeLine.hpp"
#include "Window.hpp"
#include "VKUniformBuffer.hpp"
#include "Engine.hpp"

#include <iostream>

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

	vkShader = new VKShader(vkInit->GetDevice());
	vkShader->LoadShader("../Engine/shader/vertex.vert", "../Engine/shader/fragment.frag");
	vkDescriptor = new VKDescriptor(vkInit);
	vkPipeline = new VKPipeLine(vkInit->GetDevice(), vkDescriptor->GetDescriptorSetLayout());
	vkPipeline->InitPipeLine(vkShader->GetVertexModule(), vkShader->GetFragmentModule(), vkSwapChain->GetSwapChainImageExtent(), &vkRenderPass);

	imguiManager = new ImGuiManager(vkInit, window, &vkCommandPool, &vkCommandBuffers, vkDescriptor->GetDescriptorPool(), &vkRenderPass);
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
	Texture texture(path_);
	textures.push_back(texture);
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

	auto& vkUniformBuffer = (*textures[0].GetUniformBuffers())[frameIndex];
	currentVertexMaterialDescriptorSet = &(*vkDescriptor->GetVertexMaterialDescriptorSets())[frameIndex];
	{
		//Create Vertex Material DescriptorBuffer Info
		VkDescriptorBufferInfo bufferInfo[2];
		bufferInfo[0].buffer = vkUniformBuffer;
		bufferInfo[0].offset = 0;
		bufferInfo[0].range = sizeof(glm::mat3);

		bufferInfo[1].buffer = vkUniformBuffer;
		bufferInfo[1].offset = 0;
		bufferInfo[1].range = sizeof(glm::mat3);

		//Define which resource descriptor set will point
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = *currentVertexMaterialDescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorCount = 2;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.pBufferInfo = bufferInfo;

		//Update DescriptorSet
		//DescriptorSet does not have to update every frame since it points same uniform buffer
		vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

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

	currentTextureDescriptorSet = &(*vkDescriptor->GetFragmentMaterialDescriptorSets())[frameIndex];
	{
		//Create Texture DescriptorBuffer Info
		VkDescriptorImageInfo imageInfo[2]{};
		imageInfo[0].sampler = *textures[0].GetSampler();
		imageInfo[0].imageView = *textures[0].GetImageView();
		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		imageInfo[1].sampler = *textures[1].GetSampler();
		imageInfo[1].imageView = *textures[1].GetImageView();
		imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//Define which resource descriptor set will point
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = *currentTextureDescriptorSet;
		descriptorWrite.dstBinding = 1;
		descriptorWrite.descriptorCount = 2;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pImageInfo = imageInfo;

		//Update DescriptorSet
		//DescriptorSet does not have to update every frame since it points same uniform buffer
		vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	//Update Uniform Material
	glm::mat3 mat = glm::mat3(
		1, 0, 100,
		0, 1, 200,
		0, 0, 1
	);
	//Includes Updating Uniform Function
	textures[0].Resize(mat, frameIndex);
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

	//for (int i = 0; i < textures.size(); ++i)
	//{
		//Draw Quad
		VkDeviceSize vertexBufferOffset{ 0 };
		//Bind Vertex Buffer
		vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, textures[0].GetVertexBuffer(), &vertexBufferOffset);
		//Bind Index Buffer
		vkCmdBindIndexBuffer(*currentCommandBuffer, *textures[0].GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
		//Bind Pipeline
		vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLine());
		//Bind Material DescriptorSet
		vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLineLayout(), 0, 1, currentVertexMaterialDescriptorSet, 0, nullptr);
		//Bind Texture DescriptorSet
		vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
		//Draw
		vkCmdDrawIndexed(*currentCommandBuffer, 4, 1, 0, 0, 0);
	//}

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