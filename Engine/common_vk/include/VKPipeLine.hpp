#pragma once
#include <vulkan/vulkan.hpp>

constexpr enum class POLYGON_MODE
{
	FILL = 0,
	LINE = 1,
};

class VKPipeLine
{
public:
	VKPipeLine(std::vector<VkDescriptorSetLayout>* layout_);
	~VKPipeLine();
	void InitPipeLine(VkShaderModule* vertexModule, VkShaderModule* fragmentModule, VkExtent2D* swapchainImageExtent, VkRenderPass* renderPass, POLYGON_MODE mode_);

	VkPipeline* GetPipeLine() { return &vkPipeline; };
	VkPipelineLayout* GetPipeLineLayout() { return &vkPipelineLayout; };
private:
	VkDevice* device;
	void InitPipeLineLayout();

	VkPipeline vkPipeline{ VK_NULL_HANDLE };
	VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSetLayout>* vkDescriptorSetLayout{ VK_NULL_HANDLE };
};