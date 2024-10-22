//Author: JEYOON YU
//Project: CubeEngine
//File: VKPipeLine.cpp
#include "VKPipeLine.hpp"

#include <iostream>
#include <array>

#include "../../include/Material.hpp"

VKPipeLine::VKPipeLine(VkDevice* device_, std::vector<VkDescriptorSetLayout>* layout_) : device(device_), vkDescriptorSetLayout(layout_)
{
}

VKPipeLine::~VKPipeLine()
{
	//Destroy Pipeline Layout
	vkDestroyPipelineLayout(*device, vkPipelineLayout, nullptr);
	//Destroy Pipeline
	vkDestroyPipeline(*device, vkPipeline, nullptr);
	device = nullptr;
}

void VKPipeLine::InitPipeLine(
	VkShaderModule* vertexModule,
	VkShaderModule* fragmentModule,
	VkExtent2D* swapchainImageExtent,
	VkRenderPass* renderPass,
	uint32_t stride,
	std::initializer_list<VKAttributeLayout> layout,
	VkCullModeFlags cull_,
	POLYGON_MODE mode_)
{
	//Create Pipeline Shader Stage Info
	std::array<VkPipelineShaderStageCreateInfo, 2> stageCreateInfos;

	{
		VkPipelineShaderStageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		createInfo.module = *vertexModule;
		createInfo.pName = "main";

		stageCreateInfos[0] = createInfo;
	}

	{
		VkPipelineShaderStageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		createInfo.module = *fragmentModule;
		createInfo.pName = "main";

		stageCreateInfos[1] = createInfo;
	}

	//Create Vertex Input Binding
	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = stride;
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	//Create Vertex Input Attribute
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes{};
	for (const VKAttributeLayout& attribute : layout)
	{
		VkVertexInputAttributeDescription vertexInputAttribute{};
		vertexInputAttribute.location = attribute.vertex_layout_location;
		vertexInputAttribute.binding = 0;
		vertexInputAttribute.format = attribute.format;
		vertexInputAttribute.offset = attribute.offset;

		vertexInputAttributes.push_back(vertexInputAttribute);
	}

	//{
	//	//Define Vertex Input Attribute about Position
	//	VkVertexInputAttributeDescription vertexInputAttribute{};
	//	vertexInputAttribute.location = 0;
	//	vertexInputAttribute.binding = 0;
	//	vertexInputAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	//	vertexInputAttribute.offset = offsetof(VKVertex, position);

	//	vertexInputAttributes.push_back(vertexInputAttribute);
	//}

	//{
	//	//Define Vertex Input Attribute about Quad Index
	//	VkVertexInputAttributeDescription vertexInputAttribute{};
	//	vertexInputAttribute.location = 1;
	//	vertexInputAttribute.binding = 0;
	//	vertexInputAttribute.format = VK_FORMAT_R32_SINT;
	//	vertexInputAttribute.offset = offsetof(VKVertex, index);

	//	vertexInputAttributes.push_back(vertexInputAttribute);
	//}

	//Create Pipeline Vertex Input State Info
	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
	vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
	vertexInputStateInfo.pVertexAttributeDescriptions = &vertexInputAttributes[0];

	//Create Pipeline Input Assembly State Info
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
	inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

	//Create Viewport
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = static_cast<float>(swapchainImageExtent->height);
	viewport.width = static_cast<float>(swapchainImageExtent->width);
	viewport.height = -static_cast<float>(swapchainImageExtent->height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//Create Scissor
	VkRect2D scissor{};
	scissor.extent = *swapchainImageExtent;

	//Create Viewport State Info
	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	//Create Rasterization State Info
	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
	rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//FILL, LINE, POINT...
	switch (mode_)
	{
	case POLYGON_MODE::FILL:
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		break;
	case POLYGON_MODE::LINE:
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_LINE;
		break;
	}
	//Culling
	rasterizationStateInfo.cullMode = cull_;
	rasterizationStateInfo.lineWidth = 1.0f;

	//Create Multisample State Info(MSAA)
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Create Depth Stencil State Info
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	//Create Color Blend Attachment
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	//Result does not be recorded when colorWriteMask is 0
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	//Create Color Blend State Info
	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = &colorBlendAttachment;

	//Create Dynamic State Info
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 3;
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_LINE_WIDTH,
		//For vkCmdSetPrimitiveTopology
		//VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	dynamicStateInfo.pDynamicStates = dynamicStates;

	//Create Pipeline Layout
	InitPipeLineLayout();

	//Create Graphics Pipeline Info
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = static_cast<uint32_t>(stageCreateInfos.size());
	createInfo.pStages = stageCreateInfos.data();
	createInfo.pVertexInputState = &vertexInputStateInfo;
	createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
	createInfo.pViewportState = &viewportStateInfo;
	createInfo.pRasterizationState = &rasterizationStateInfo;
	createInfo.pMultisampleState = &multisampleCreateInfo;
	createInfo.pDepthStencilState = &depthStencilCreateInfo;
	createInfo.pColorBlendState = &colorBlendStateInfo;
	createInfo.layout = vkPipelineLayout;
	createInfo.renderPass = *renderPass;
	createInfo.pDynamicState = &dynamicStateInfo;

	//Create Graphics Pipeline
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &vkPipeline);
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

			throw std::runtime_error{ "Graphics Pipeline Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKPipeLine::~VKPipeLine();
		std::exit(EXIT_FAILURE);
	}
}

void VKPipeLine::InitPipeLineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ThreeDimension::VertexLightingUniform);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	//Create Pipeline Layout Info
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayout->size());
	//createInfo.pSetLayouts = &(*vkDescriptorSetLayout)[0];
	createInfo.pSetLayouts = vkDescriptorSetLayout->data();
	createInfo.pPushConstantRanges = &pushConstantRange;
	createInfo.pushConstantRangeCount = 1;

	//Create Pipeline Layout
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreatePipelineLayout(*device, &createInfo, nullptr, &vkPipelineLayout);
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

			throw std::runtime_error{ "Graphics Pipeline Layout Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKPipeLine::~VKPipeLine();
		std::exit(EXIT_FAILURE);
	}
}
