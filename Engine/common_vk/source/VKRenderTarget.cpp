//Author: JEYOON YU
//Project: CubeEngine
//File: VKRenderTarget.cpp
#include "VKRenderTarget.hpp"
#include "VKHelper.hpp"

#include "VKInit.hpp"
#include "VKSwapChain.hpp"

VKRenderTarget::VKRenderTarget(VKInit* vkInit, VKSwapChain* vkSwapChain) : m_vkInit(vkInit), m_vkSwapChain(vkSwapChain)
{
	GetMaxUsableSampleCount();
	CreateColorResources();
	CreateDepthBuffer();
}

VKRenderTarget::~VKRenderTarget()
{
	//Destroy MSAA
	vkDestroyImageView(*m_vkInit->GetDevice(), m_colorImageView, nullptr);
	vkFreeMemory(*m_vkInit->GetDevice(), m_colorImageMemory, nullptr);
	vkDestroyImage(*m_vkInit->GetDevice(), m_colorImage, nullptr);

	//Destroy Depth Buffering
	vkDestroyImageView(*m_vkInit->GetDevice(), m_depthImageView, nullptr);
	vkFreeMemory(*m_vkInit->GetDevice(), m_depthImageMemory, nullptr);
	vkDestroyImage(*m_vkInit->GetDevice(), m_depthImage, nullptr);
}

void VKRenderTarget::GetMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(*m_vkInit->GetPhysicalDevice(), &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_64_BIT; return; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_32_BIT; return; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_16_BIT; return; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_8_BIT; return; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_4_BIT; return; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_2_BIT; return; }
	m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
}

void VKRenderTarget::CreateColorResources()
{
	m_imageFormat = m_vkInit->SetSurfaceFormat().format;

	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = m_imageFormat;
		createInfo.extent = { m_vkSwapChain->GetSwapChainImageExtent()->width, m_vkSwapChain->GetSwapChainImageExtent()->height, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.samples = m_msaaSamples;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying and shader
		createInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		//Create image
		VKHelper::ThrowIfFailed(vkCreateImage(*m_vkInit->GetDevice(), &createInfo, nullptr, &m_colorImage));

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements{};
		//Get Memory Requirements for Image
		vkGetImageMemoryRequirements(*m_vkInit->GetDevice(), m_colorImage, &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Select memory type which has fast access from GPU
		allocateInfo.memoryTypeIndex = VKHelper::FindMemoryTypeIndex(*m_vkInit->GetPhysicalDevice(), requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//Allocate Memory
		VKHelper::ThrowIfFailed(vkAllocateMemory(*m_vkInit->GetDevice(), &allocateInfo, nullptr, &m_colorImageMemory));

		//Bind Image and Memory
		VKHelper::ThrowIfFailed(vkBindImageMemory(*m_vkInit->GetDevice(), m_colorImage, m_colorImageMemory, 0));
	}

	//To access image from graphics pipeline, Image View is needed
	// Create ImageView Info
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = m_colorImage;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = m_imageFormat;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	// Create ImageView
	VKHelper::ThrowIfFailed(vkCreateImageView(*m_vkInit->GetDevice(), &createInfo, nullptr, &m_colorImageView));
}

// Depth
VkFormat VKRenderTarget::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(*m_vkInit->GetPhysicalDevice(), format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	throw std::runtime_error("Failed To Find Supported Format!");
};

VkFormat VKRenderTarget::FindDepthFormat() const
{
	return FindSupportedFormat
	(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool VKRenderTarget::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VKRenderTarget::CreateDepthBuffer()
{
	m_depthFormat = FindDepthFormat();

	{
		//Define an image to create
		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = m_depthFormat;
		createInfo.extent = { m_vkSwapChain->GetSwapChainImageExtent()->width, m_vkSwapChain->GetSwapChainImageExtent()->height, 1 };
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = m_msaaSamples;
		//Use Optimal Tiling to make GPU effectively process image
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//Usage for copying and shader
		createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//Create image
		VKHelper::ThrowIfFailed(vkCreateImage(*m_vkInit->GetDevice(), &createInfo, nullptr, &m_depthImage));

		//Declare a variable which will take memory requirements
		VkMemoryRequirements requirements{};
		//Get Memory Requirements for Image
		vkGetImageMemoryRequirements(*m_vkInit->GetDevice(), m_depthImage, &requirements);

		//Create Memory Allocation Info
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = requirements.size;
		//Select memory type which has fast access from GPU
		allocateInfo.memoryTypeIndex = VKHelper::FindMemoryTypeIndex(*m_vkInit->GetPhysicalDevice(), requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//Allocate Memory
		VKHelper::ThrowIfFailed(vkAllocateMemory(*m_vkInit->GetDevice(), &allocateInfo, nullptr, &m_depthImageMemory));

		//Bind Image and Memory
		VKHelper::ThrowIfFailed(vkBindImageMemory(*m_vkInit->GetDevice(), m_depthImage, m_depthImageMemory, 0));
	}

	//To access image from graphics pipeline, Image View is needed
	//Create ImageView Info
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = m_depthImage;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = m_depthFormat;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	//Create ImageView
	VKHelper::ThrowIfFailed(vkCreateImageView(*m_vkInit->GetDevice(), &createInfo, nullptr, &m_depthImageView));
}
