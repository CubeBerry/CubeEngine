#include "ImGuiManager.hpp"

#include <iostream>
#include "VKInit.hpp"
#include "VKSwapChain.hpp"
#include "VKDescriptor.hpp"
#include "VKPipeLine.hpp"
#include "Window.hpp"
#include "Engine.hpp"

ImGuiManager::ImGuiManager(VKInit* init_, VKSwapChain* chain_, VkPipeline* pipeline_, SDL_Window* window_, std::array<VkCommandBuffer, 2> buffers_) : vkInit(init_), vkSwapChain(chain_), pipeline(pipeline_), window(window_), vkCommandBuffers(buffers_)
{
	//VkSurfaceFormatKHR surfaceFormat = vkInit->SetSurfaceFormat();

	////Create Attachment Description
	//VkAttachmentDescription attachmentDescription{};
	//attachmentDescription.format = surfaceFormat.format;
	//attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	//attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	//attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	////Define which attachment should subpass refernece of renderpass
	//VkAttachmentReference colorAttachmentReference{};
	////attachment == Index of VkAttachmentDescription array
	//colorAttachmentReference.attachment = 0;
	//colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	////Create Subpass Description
	//VkSubpassDescription subpassDescription{};
	//subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//subpassDescription.colorAttachmentCount = 1;
	//subpassDescription.pColorAttachments = &colorAttachmentReference;

	//VkSubpassDependency subpassDependency{};
	//subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	//subpassDependency.dstSubpass = 0;
	//subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subpassDependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	////Create Renderpass Info
	//VkRenderPassCreateInfo createInfo{};
	//createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//createInfo.attachmentCount = 1;
	//createInfo.pAttachments = &attachmentDescription;
	//createInfo.subpassCount = 1;
	//createInfo.pSubpasses = &subpassDescription;
	//createInfo.dependencyCount = 1;
	//createInfo.pDependencies = &subpassDependency;

	////Create Renderpass
	//try
	//{
	//	VkResult result{ VK_SUCCESS };
	//	result = vkCreateRenderPass(*vkInit->GetDevice(), &createInfo, nullptr, &renderPass);
	//	if (result != VK_SUCCESS)
	//	{
	//		switch (result)
	//		{
	//		case VK_ERROR_OUT_OF_HOST_MEMORY:
	//			std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	//			break;
	//		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	//			std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	//			break;
	//		default:
	//			break;
	//		}
	//		std::cout << std::endl;

	//		throw std::runtime_error{ "RenderPass Creation Failed" };
	//	}
	//}
	//catch (std::exception& e)
	//{
	//	std::cerr << e.what() << std::endl;
	//	ImGuiManager::~ImGuiManager();
	//	std::exit(EXIT_FAILURE);
	//}

	//Initialize();
}

ImGuiManager::~ImGuiManager()
{
	Shutdown();
}

void ImGuiManager::Initialize(VkDescriptorPool* dpool_, VkRenderPass* pass_, VkCommandPool cpool_)
{
	//{
	//	VkDescriptorPoolSize poolSize[] =
	//	{
	//		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 }
	//	};
	//	VkDescriptorPoolCreateInfo createInfo = {};
	//	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	//	createInfo.maxSets = 2;
	//	createInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSize);
	//	createInfo.pPoolSizes = poolSize;
	//	//Create DescriptorPool
	//	try
	//	{
	//		VkResult result{ VK_SUCCESS };
	//		result = vkCreateDescriptorPool(*vkInit->GetDevice(), &createInfo, nullptr, &vkDescriptorPool);
	//		if (result != VK_SUCCESS)
	//		{
	//			switch (result)
	//			{
	//			case VK_ERROR_OUT_OF_HOST_MEMORY:
	//				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	//				break;
	//			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	//				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	//				break;
	//			default:
	//				break;
	//			}
	//			std::cout << std::endl;

	//			throw std::runtime_error{ "Descriptor Pool Creation Failed" };
	//		}
	//	}
	//	catch (std::exception& e)
	//	{
	//		std::cerr << e.what() << std::endl;
	//		ImGuiManager::~ImGuiManager();
	//		std::exit(EXIT_FAILURE);
	//	}
	//}

	//{
	//	//Create command pool info
	//	VkCommandPoolCreateInfo commandPoolInfo{};
	//	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	//	commandPoolInfo.queueFamilyIndex = *vkInit->GetQueueFamilyIndex();

	//	//Create command pool
	//	try
	//	{
	//		VkResult result{ VK_SUCCESS };
	//		result = vkCreateCommandPool(*vkInit->GetDevice(), &commandPoolInfo, nullptr, &vkCommandPool);
	//		if (result != VK_SUCCESS)
	//		{
	//			switch (result)
	//			{
	//			case VK_ERROR_OUT_OF_HOST_MEMORY:
	//				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	//				break;
	//			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	//				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	//				break;
	//			default:
	//				break;
	//			}
	//			std::cout << std::endl;

	//			throw std::runtime_error{ "Command Pool Creation Failed" };
	//		}
	//	}
	//	catch (std::exception& e)
	//	{
	//		std::cerr << e.what() << std::endl;
	//		ImGuiManager::~ImGuiManager();
	//		std::exit(EXIT_FAILURE);
	//	}
	//}

	//{
	//	//Create command buffer info
	//	VkCommandBufferAllocateInfo allocateInfo{};
	//	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//	allocateInfo.commandPool = vkCommandPool;
	//	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//	allocateInfo.commandBufferCount = 2;

	//	//Create command buffer
	//	try
	//	{
	//		VkResult result{ VK_SUCCESS };
	//		result = vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &vkCommandBuffers[0]);
	//		if (result != VK_SUCCESS)
	//		{
	//			switch (result)
	//			{
	//			case VK_ERROR_OUT_OF_HOST_MEMORY:
	//				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	//				break;
	//			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	//				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	//				break;
	//			default:
	//				break;
	//			}
	//			std::cout << std::endl;

	//			throw std::runtime_error{ "Command Buffer Creation Failed" };
	//		}
	//	}
	//	catch (std::exception& e)
	//	{
	//		std::cerr << e.what() << std::endl;
	//		ImGuiManager::~ImGuiManager();
	//		std::exit(EXIT_FAILURE);
	//	}
	//}

	//{
	//	//Create framebuffer info
	//	VkFramebufferCreateInfo createInfo{};
	//	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//	createInfo.renderPass = renderPass;
	//	createInfo.attachmentCount = 1;
	//	createInfo.width = vkSwapChain->GetSwapChainImageExtent()->width;
	//	createInfo.height = vkSwapChain->GetSwapChainImageExtent()->height;
	//	createInfo.layers = 1;

	//	//Allocate memory for framebuffers
	//	vkFrameBuffers.resize(vkSwapChain->GetSwapChainImageViews()->size());

	//	try
	//	{
	//		for (auto i = 0; i != vkSwapChain->GetSwapChainImageViews()->size(); ++i)
	//		{
	//			createInfo.pAttachments = &(*vkSwapChain->GetSwapChainImageViews())[i];

	//			//Create framebuffer
	//			VkResult result{ VK_SUCCESS };
	//			result = vkCreateFramebuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkFrameBuffers[i]);
	//			if (result != VK_SUCCESS)
	//			{
	//				switch (result)
	//				{
	//				case VK_ERROR_OUT_OF_HOST_MEMORY:
	//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	//					break;
	//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
	//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	//					break;
	//				default:
	//					break;
	//				}
	//				std::cout << std::endl;

	//				throw std::runtime_error{ "Framebuffer Creation Failed" };
	//			}
	//		}
	//	}
	//	catch (std::exception& e)
	//	{
	//		std::cerr << e.what() << std::endl;
	//		ImGuiManager::~ImGuiManager();
	//		std::exit(EXIT_FAILURE);
	//	}
	//}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui_ImplSDL2_InitForVulkan(window);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = *vkInit->GetInstance();
	initInfo.PhysicalDevice = *vkInit->GetPhysicalDevice();
	initInfo.Device = *vkInit->GetDevice();
	initInfo.Queue = *vkInit->GetQueue();
	initInfo.QueueFamily = *vkInit->GetQueueFamilyIndex();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	//initInfo.DescriptorPool = vkDescriptorPool;
	initInfo.DescriptorPool = *dpool_;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = 2;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo, *pass_);

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	{
		VkCommandBuffer commandBuffer{};
		//Create command buffer info
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = cpool_;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		//Create command buffer
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkAllocateCommandBuffers(*vkInit->GetDevice(), &allocateInfo, &commandBuffer);
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
			ImGuiManager::~ImGuiManager();
			std::exit(EXIT_FAILURE);
		}

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		//End Command Buffer
		vkEndCommandBuffer(commandBuffer);

		//Create Submit Info
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		//Submit Queue to Command Buffer
		vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		//Wait until all submitted command buffers are handled
		vkDeviceWaitIdle(*vkInit->GetDevice());

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void ImGuiManager::FeedEvent(const SDL_Event& event_)
{
	ImGui_ImplSDL2_ProcessEvent(&event_);
}

void ImGuiManager::Begin()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End(uint32_t index_)
{
	ImGui::Render();

	////Reset command buffer
	//vkResetCommandBuffer(vkCommandBuffers[index_], 0);

	////Create command buffer begin info
	//VkCommandBufferBeginInfo beginInfo{};
	//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	////Begin command buffer
	//vkBeginCommandBuffer(vkCommandBuffers[index_], &beginInfo);

	////Set clear color
	//VkClearValue clearValue{};
	//clearValue.color.float32[0] = 1.0f;	//R
	//clearValue.color.float32[1] = 0.0f;	//G
	//clearValue.color.float32[2] = 1.0f;	//B
	//clearValue.color.float32[3] = 1.0f;	//A

	////Create renderpass begin info
	//VkRenderPassBeginInfo renderpassBeginInfo{};
	//renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//renderpassBeginInfo.renderPass = renderPass;
	//renderpassBeginInfo.framebuffer = vkFrameBuffers[index_];
	//renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
	//renderpassBeginInfo.clearValueCount = 1;
	//renderpassBeginInfo.pClearValues = &clearValue;

	////Begin renderpass
	//vkCmdBeginRenderPass(vkCommandBuffers[index_], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCommandBuffers[index_]);

	////End renderpass
	//vkCmdEndRenderPass(vkCommandBuffers[index_]);

	////End command buffer
	//vkEndCommandBuffer(vkCommandBuffers[index_]);
}

void ImGuiManager::Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

//void ImGuiManager::CleanSwapChain()
//{
//	//Destroy FrameBuffer
//	for (auto& framebuffer : vkFrameBuffers)
//	{
//		vkDestroyFramebuffer(*vkInit->GetDevice(), framebuffer, nullptr);
//	}
//	//Destroy ImageView
//	for (auto& imageView : *vkSwapChain->GetSwapChainImageViews())
//	{
//		vkDestroyImageView(*vkInit->GetDevice(), imageView, nullptr);
//	}
//	//Destroy SwapChain
//	vkDestroySwapchainKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), nullptr);
//}
//
//void ImGuiManager::RecreateSwapChain(Window* window_)
//{
//	int width = 0, height = 0;
//	while (window_->GetMinimized())
//	{
//		SDL_PumpEvents();
//	}
//	SDL_GL_GetDrawableSize(window, &width, &height);
//
//	vkDeviceWaitIdle(*vkInit->GetDevice());
//
//	CleanSwapChain();
//
//	vkSwapChain->InitSwapChain();
//	vkSwapChain->InitSwapChainImage(&vkCommandBuffers[0]);
//	vkSwapChain->InitSwapChainImageView();
//
//	//Create framebuffer info
//	VkFramebufferCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	createInfo.renderPass = renderPass;
//	createInfo.attachmentCount = 1;
//	createInfo.width = vkSwapChain->GetSwapChainImageExtent()->width;
//	createInfo.height = vkSwapChain->GetSwapChainImageExtent()->height;
//	createInfo.layers = 1;
//
//	//Allocate memory for framebuffers
//	vkFrameBuffers.resize(vkSwapChain->GetSwapChainImageViews()->size());
//
//	try
//	{
//		for (auto i = 0; i != vkSwapChain->GetSwapChainImageViews()->size(); ++i)
//		{
//			createInfo.pAttachments = &(*vkSwapChain->GetSwapChainImageViews())[i];
//
//			//Create framebuffer
//			VkResult result{ VK_SUCCESS };
//			result = vkCreateFramebuffer(*vkInit->GetDevice(), &createInfo, nullptr, &vkFrameBuffers[i]);
//			if (result != VK_SUCCESS)
//			{
//				switch (result)
//				{
//				case VK_ERROR_OUT_OF_HOST_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
//					break;
//				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//					std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
//					break;
//				default:
//					break;
//				}
//				std::cout << std::endl;
//
//				throw std::runtime_error{ "Framebuffer Creation Failed" };
//			}
//		}
//	}
//	catch (std::exception& e)
//	{
//		std::cerr << e.what() << std::endl;
//		ImGuiManager::~ImGuiManager();
//		std::exit(EXIT_FAILURE);
//	}
//}