//Author: JEYOON YU
//Project: CubeEngine
//File: VKSwapChain.cpp
#include "VKSwapChain.hpp"
#include "VKInit.hpp"
#include "VKHelper.hpp"

#include <iostream>

VKSwapChain::VKSwapChain(VKInit* init_, VkCommandPool* pool_) : vkInit(init_), vkCommandPool(pool_)
{
	InitSwapChain();
	InitSwapChainImage();
	InitFence();
	InitSemaphore();
	InitSwapChainImageView();
}

VKSwapChain::~VKSwapChain()
{
	//Destroy SwapChain
	vkDestroySwapchainKHR(*vkInit->GetDevice(), vkSwapChain, nullptr);
	//Destroy Fences
	for (auto& fence : vkFences)
	{
		vkDestroyFence(*vkInit->GetDevice(), fence, nullptr);
	}
	//Destroy Semaphore
	for (auto& semaphores : vkSemaphores)
	{
		for (auto& semaphore : semaphores)
			vkDestroySemaphore(*vkInit->GetDevice(), semaphore, nullptr);
	}
	//Destroy ImageView
	for (auto& imageView : vkSwapChainImageViews)
	{
		vkDestroyImageView(*vkInit->GetDevice(), imageView, nullptr);
	}
}

void VKSwapChain::InitSwapChain()
{
	//Get Surface Capabilities
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VKHelper::ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*vkInit->GetPhysicalDevice(), *vkInit->GetSurface(), &surfaceCapabilities));

	//Find available composite alpha mode
	VkCompositeAlphaFlagBitsKHR compositeAlpha{ static_cast<VkCompositeAlphaFlagBitsKHR>(0) };
	for (auto i = 0; i != 32; ++i)
	{
		VkCompositeAlphaFlagBitsKHR flag = static_cast<VkCompositeAlphaFlagBitsKHR>(0x1 << i);
		if (surfaceCapabilities.supportedCompositeAlpha & flag)
		{
			compositeAlpha = flag;
			break;
		}
	}

	//Save swapchain image's extent
	swapchainImageExtent = surfaceCapabilities.currentExtent;

	//Create Swapchain Create Info
	VkSurfaceFormatKHR surfaceFormat = vkInit->SetSurfaceFormat();

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = *vkInit->GetSurface();
	swapchainInfo.minImageCount = 2;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainInfo.compositeAlpha = compositeAlpha;
	swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	//Create Swapchain
	VKHelper::ThrowIfFailed(vkCreateSwapchainKHR(*vkInit->GetDevice(), &swapchainInfo, nullptr, &vkSwapChain));
}

void VKSwapChain::InitSwapChainImage()
{
	//if pSwapchainImages == nullptr -> returns numbers of all available swapchain images
	uint32_t count{ 0 };
	VKHelper::ThrowIfFailed(vkGetSwapchainImagesKHR(*vkInit->GetDevice(), vkSwapChain, &count, nullptr));

	//Get swapchain images to vector
	vkSwapChainImages.resize(count);
	VKHelper::ThrowIfFailed(vkGetSwapchainImagesKHR(*vkInit->GetDevice(), vkSwapChain, &count, &vkSwapChainImages[0]));

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = *vkCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer_{};
	VKHelper::ThrowIfFailed(vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &commandBuffer_));

	VKHelper::ThrowIfFailed(vkBeginCommandBuffer(commandBuffer_, &beginInfo));

	//Change whole swapchain's image layout from UNDEFINED to PRESENT_SRC
	std::vector<VkImageMemoryBarrier> barriers;
	for (auto& image : vkSwapChainImages)
	{
		VkImageMemoryBarrier barrier{};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		barriers.push_back(barrier);
	}

	//Record command to command buffer which runs defined barrier
	vkCmdPipelineBarrier(commandBuffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), &barriers[0]);

	//End command buffer
	VKHelper::ThrowIfFailed(vkEndCommandBuffer(commandBuffer_));

	//Create submit info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer_;

	//Submit command to queue
	VKHelper::ThrowIfFailed(vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	//Wait until all submitted command buffers are handled
	VKHelper::ThrowIfFailed(vkDeviceWaitIdle(*vkInit->GetDevice()));

	//Deallocate Command Buffers
	vkFreeCommandBuffers(*vkInit->GetDevice(), *vkCommandPool, 1, &commandBuffer_);
}

void VKSwapChain::InitSwapChainImageView()
{
	//ImageView is needed for accessing image at graphics pipeline

	VkSurfaceFormatKHR surfaceFormat = vkInit->SetSurfaceFormat();

	//Create ImageView Info
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = surfaceFormat.format;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	//Allocate memory for imageview
	vkSwapChainImageViews.resize(vkSwapChainImages.size());

	for (auto i = 0; i != vkSwapChainImages.size(); ++i)
	{
		//Define image to create imageview
		createInfo.image = vkSwapChainImages[i];

		//Create imageview
		VKHelper::ThrowIfFailed(vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkSwapChainImageViews[i]));
	}
}

void VKSwapChain::InitFence()
{
	//Create fence info
	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	//Create fence
	for (auto& fence : vkFences)
	{
		VKHelper::ThrowIfFailed(vkCreateFence(*vkInit->GetDevice(), &createInfo, nullptr, &fence));
	}
}

void VKSwapChain::InitSemaphore()
{
	//Create semaphore info
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	//Create semaphore
	for (auto& semaphores : vkSemaphores)
	{
		for (auto& semaphore : semaphores)
		{
			VKHelper::ThrowIfFailed(vkCreateSemaphore(*vkInit->GetDevice(), &createInfo, nullptr, &semaphore));
		}
	}
}
