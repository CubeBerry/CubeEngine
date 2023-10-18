#include "VKPipeLine.hpp"
#include "Vertex.hpp"

#include <iostream>
#include <array>

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

void VKPipeLine::InitPipeLine(VkShaderModule* vertexModule, VkShaderModule* fragmentModule, VkExtent2D* swapchainImageExtent, VkRenderPass* renderPass)
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
	vertexInputBinding.stride = sizeof(Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	//Create Vertex Input Attribute
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes{};

	{
		//Define Vertex Input Attribute about Position
		VkVertexInputAttributeDescription vertexInputAttribute{};
		vertexInputAttribute.location = 0;
		vertexInputAttribute.binding = 0;
		vertexInputAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttribute.offset = offsetof(Vertex, position);

		vertexInputAttributes.push_back(vertexInputAttribute);
	}

	{
		//Define Vertex Input Attribute about Color
		VkVertexInputAttributeDescription vertexInputAttribute{};
		vertexInputAttribute.location = 1;
		vertexInputAttribute.binding = 0;
		vertexInputAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttribute.offset = offsetof(Vertex, color);

		vertexInputAttributes.push_back(vertexInputAttribute);
	}

	{
		//Define Vertex Input Attribute about Texture
		VkVertexInputAttributeDescription vertexInputAttribute{};
		vertexInputAttribute.location = 2;
		vertexInputAttribute.binding = 0;
		vertexInputAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttribute.offset = offsetof(Vertex, uv);

		vertexInputAttributes.push_back(vertexInputAttribute);
	}

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

	//Create Viewport
	VkViewport viewport{};
	viewport.width = static_cast<float>(swapchainImageExtent->width);
	viewport.height = static_cast<float>(swapchainImageExtent->height);
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
	rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	//Culling
	rasterizationStateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationStateInfo.lineWidth = 1.0f;

	//Create Multisample State Info(MSAA)
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Create Depth Stencil State Info
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	//Create Color Blend Attachment
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	//Result does not be recorded when colorWriteMask is 0
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT;

	//Create Color Blend State Info
	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = &colorBlendAttachment;

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
	//Create Pipeline Layout Info
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayout->size());
	createInfo.pSetLayouts = &(*vkDescriptorSetLayout)[0];

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
