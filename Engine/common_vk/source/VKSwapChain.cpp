#include "VKSwapChain.hpp"
#include "VKInit.hpp"

#include <iostream>

VKSwapChain::VKSwapChain(VKInit* init_) : vkInit(init_)
{
	InitSwapChain();
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
	for (auto& semaphore : vkSemaphores)
	{
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
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*vkInit->GetPhysicalDevice(), *vkInit->GetSurface(), &surfaceCapabilities);

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
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateSwapchainKHR(*vkInit->GetDevice(), &swapchainInfo, nullptr, &vkSwapChain);
		if (result != VK_SUCCESS)
		{
			std::cout << std::endl;
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
				break;
			case VK_ERROR_DEVICE_LOST:
				std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
				break;
			case VK_ERROR_SURFACE_LOST_KHR:
				std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
				break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				std::cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;
				break;
			case VK_ERROR_INITIALIZATION_FAILED:
				std::cout << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
				break;
			default:
				break;
			}
			std::cout << std::endl;

			throw std::runtime_error{ "SwapChain Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSwapChain::~VKSwapChain();
		std::exit(EXIT_FAILURE);
	}
}

void VKSwapChain::InitSwapChainImage(VkCommandBuffer* commandBuffer_)
{
	//if pSwapchainImages == nullptr -> returns numbers of all available swapchain images
	uint32_t count{ 0 };
	vkGetSwapchainImagesKHR(*vkInit->GetDevice(), vkSwapChain, &count, nullptr);

	//Get swapchain images to vector
	vkSwapChainImages.resize(count);
	vkGetSwapchainImagesKHR(*vkInit->GetDevice(), vkSwapChain, &count, &vkSwapChainImages[0]);

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	vkBeginCommandBuffer(*commandBuffer_, &beginInfo);

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
	vkCmdPipelineBarrier(*commandBuffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), &barriers[0]);

	//End command buffer
	vkEndCommandBuffer(*commandBuffer_);

	//Create submit info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffer_;

	//Submit command to queue
	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//Wait until all submitted command buffers are handled
	vkDeviceWaitIdle(*vkInit->GetDevice());
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

	try
	{
		for (auto i = 0; i != vkSwapChainImages.size(); ++i)
		{
			//Define image to create imageview
			createInfo.image = vkSwapChainImages[i];

			//Create imageview
			VkResult result{ VK_SUCCESS };
			result = vkCreateImageView(*vkInit->GetDevice(), &createInfo, nullptr, &vkSwapChainImageViews[i]);
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

				throw std::runtime_error{ "Swapchain Image View Creation Failed" };
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSwapChain::~VKSwapChain();
		std::exit(EXIT_FAILURE);
	}
}

void VKSwapChain::InitFence()
{
	//Create fence info
	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	//Create fence
	try
	{
		for (auto& fence : vkFences)
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateFence(*vkInit->GetDevice(), &createInfo, nullptr, &fence);
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

				throw std::runtime_error{ "Fence Creation Failed" };
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSwapChain::~VKSwapChain();
		std::exit(EXIT_FAILURE);
	}
}

void VKSwapChain::InitSemaphore()
{
	//Create semaphore info
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	//Create semaphore
	try
	{
		for (auto& semaphore : vkSemaphores)
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateSemaphore(*vkInit->GetDevice(), &createInfo, nullptr, &semaphore);
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

				throw std::runtime_error{ "Semaphore Creation Failed" };
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKSwapChain::~VKSwapChain();
		std::exit(EXIT_FAILURE);
	}
}
