//Author: JEYOON YU
//Project: CubeEngine
//File: VKPipeLine.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <initializer_list>

struct VKAttributeLayout
{
    // Is this the 1st, 2nd, 3rd... (0, 1, 2...) "in attribute" of the vertex shader?
    uint32_t vertex_layout_location = 0;
    VkFormat format;
    // byte offset to read the very 1st attribute
    // should be 0 for parallel array and struct of arrays
    // should be offsetof(Vertex, field) for array of structs
    uint32_t offset = 0;
};

enum class POLYGON_MODE
{
	FILL = 0,
	LINE = 1,
};

class VKPipeLine
{
public:
	VKPipeLine(VkDevice* device_, std::vector<VkDescriptorSetLayout>* layout_);
	~VKPipeLine();
	void InitPipeLine(
		VkShaderModule* vertexModule,
		VkShaderModule* fragmentModule,
		VkExtent2D* swapchainImageExtent,
		VkRenderPass* renderPass,
		uint32_t stride,
		std::initializer_list<VKAttributeLayout> layout,
		VkSampleCountFlagBits samples,
		VkPrimitiveTopology primitiveTopology,
		VkCullModeFlags cull_,
		POLYGON_MODE mode_,
		bool isPushConstant,
		uint32_t pushConstantSize = 0,
		VkShaderStageFlagBits pushConstantShaderBit = VK_SHADER_STAGE_VERTEX_BIT
	);

	VkPipeline* GetPipeLine() { return &vkPipeline; };
	VkPipelineLayout* GetPipeLineLayout() { return &vkPipelineLayout; };
private:
	VkDevice* device;
	void InitPipeLineLayout(bool isPushConstant, uint32_t size, VkShaderStageFlagBits bit);

	VkPipeline vkPipeline{ VK_NULL_HANDLE };
	VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSetLayout>* vkDescriptorSetLayout{ VK_NULL_HANDLE };
};