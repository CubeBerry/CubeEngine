//Author: JEYOON YU
//Project: CubeEngine
//File: VKRenderTarget.hpp
#pragma once
#include <filesystem>
#include <vulkan/vulkan.hpp>

class VKInit;
class VKSwapChain;

class VKRenderTarget
{
public:
	VKRenderTarget(VKInit* vkInit, VKSwapChain* vkSwapChain);
	~VKRenderTarget();

	// MSAA
	VkSampleCountFlagBits GetMSAASamples() const { return m_msaaSamples; }
	VkImageView GetColorImageView() const { return m_colorImageView; }

	// Depth
	VkFormat GetDepthFormat() const { return m_depthFormat; }
	VkImageView GetDepthImageView() const { return m_depthImageView; }
private:
	// Common
	VKInit* m_vkInit{ nullptr };
	VKSwapChain* m_vkSwapChain{ nullptr };

	// MSAA
	//This function should be called after VKInit's Initialize() function
	void GetMaxUsableSampleCount();
	void CreateColorResources();

	VkImage m_colorImage;
	VkDeviceMemory m_colorImageMemory;
	VkImageView m_colorImageView;
	VkFormat m_imageFormat;
	VkSampleCountFlagBits m_msaaSamples{ VK_SAMPLE_COUNT_1_BIT };

	// Depth
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	VkFormat FindDepthFormat() const;
	bool HasStencilComponent(VkFormat format);
	void CreateDepthBuffer();

	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;
	VkFormat m_depthFormat;
};
