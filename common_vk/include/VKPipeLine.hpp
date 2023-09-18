#pragma once
#include <vulkan/vulkan.hpp>

class VKPipeLine
{
public:
	VKPipeLine(VkDevice* device_, std::vector<VkDescriptorSetLayout>* layout_);
	~VKPipeLine();
	void InitPipeLine(VkShaderModule* vertexModule, VkShaderModule* fragmentModule, VkExtent2D* swapchainImageExtent, VkRenderPass* renderPass);

	VkPipeline* GetPipeLine() { return &vkPipeline; };
	VkPipelineLayout* GetPipeLineLayout() { return &vkPipelineLayout; };
private:
	VkDevice* device;
	void InitPipeLineLayout();

	VkPipeline vkPipeline{ VK_NULL_HANDLE };
	VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSetLayout>* vkDescriptorSetLayout{ VK_NULL_HANDLE };
};