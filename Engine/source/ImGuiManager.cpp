//Author: JEYOON YU
//Project: CubeEngine
//File: ImGuiManager.cpp
#include "ImGuiManager.hpp"

#include <iostream>
#include "VKInit.hpp"
#include "VKSwapChain.hpp"
#include "VKDescriptor.hpp"
#include "VKPipeLine.hpp"
#include "Window.hpp"
#include "Engine.hpp"

ImGuiManager::ImGuiManager(VKInit* init_, SDL_Window* window_, VkCommandPool* cpool_, std::array<VkCommandBuffer, 2>* cbuffers_, VkDescriptorPool* dpool_, VkRenderPass* pass_)
{
	vkInit = init_;
	window = window_;
	vkCommandPool = *cpool_;
	vkCommandBuffers = *cbuffers_;
	vkDescriptorPool = *dpool_;
	renderPass = *pass_;

	Initialize();
}

ImGuiManager::~ImGuiManager()
{
	Shutdown();
}

void ImGuiManager::Initialize()
{
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
	initInfo.DescriptorPool = vkDescriptorPool;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = 2;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo, renderPass);

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
		allocateInfo.commandPool = vkCommandPool;
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

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCommandBuffers[index_]);
}

void ImGuiManager::Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}
