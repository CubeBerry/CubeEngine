//Author: JEYOON YU
//Project: CubeEngine
//File: VKImGuiManager.cpp
#include "VKImGuiManager.hpp"

#include <iostream>
#include "VKInit.hpp"
#include "VKSwapChain.hpp"
#include "VKDescriptor.hpp"
#include "VKPipeLine.hpp"

VKImGuiManager::VKImGuiManager(VKInit* init_, SDL_Window* window_, VkCommandPool* cpool_, std::array<VkCommandBuffer, 2>* cbuffers_, VkDescriptorPool* dpool_, VkRenderPass* pass_, VkSampleCountFlagBits samples_)
{
	vkCommandPool = *cpool_;
	vkCommandBuffers = *cbuffers_;
	vkDescriptorPool = *dpool_;
	renderPass = *pass_;

	Initialize(init_, window_, samples_);
}

VKImGuiManager::~VKImGuiManager()
{
	Shutdown();
}

void VKImGuiManager::Initialize(VKInit* init_, SDL_Window* window_, VkSampleCountFlagBits samples)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui_ImplSDL3_InitForVulkan(window_);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = *init_->GetInstance();
	initInfo.PhysicalDevice = *init_->GetPhysicalDevice();
	initInfo.Device = *init_->GetDevice();
	initInfo.Queue = *init_->GetQueue();
	initInfo.QueueFamily = *init_->GetQueueFamilyIndex();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = vkDescriptorPool;
	initInfo.RenderPass = renderPass;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = 2;
	initInfo.MSAASamples = samples;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo);
	ImGui_ImplVulkan_CreateFontsTexture();
	//ImGui_ImplVulkan_Init(&initInfo, renderPass);

	////Create command buffer begin info
	//VkCommandBufferBeginInfo beginInfo{};
	//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	////Begin command buffer
	//{
	//	VkCommandBuffer commandBuffer{};
	//	//Create command buffer info
	//	VkCommandBufferAllocateInfo allocateInfo{};
	//	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//	allocateInfo.commandPool = vkCommandPool;
	//	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//	allocateInfo.commandBufferCount = 1;

	//	//Create command buffer
	//	try
	//	{
	//		VkResult result{ VK_SUCCESS };
	//		result = vkAllocateCommandBuffers(*init_->GetDevice(), &allocateInfo, &commandBuffer);
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
	//		VKImGuiManager::~VKImGuiManager();
	//		std::exit(EXIT_FAILURE);
	//	}

	//	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	//	ImGui_ImplVulkan_CreateFontsTexture();
	//	//End Command Buffer
	//	vkEndCommandBuffer(commandBuffer);

	//	//Create Submit Info
	//	VkSubmitInfo submitInfo{};
	//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//	submitInfo.commandBufferCount = 1;
	//	submitInfo.pCommandBuffers = &commandBuffer;

	//	//Submit Queue to Command Buffer
	//	vkQueueSubmit(*init_->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	//	//Wait until all submitted command buffers are handled
	//	vkDeviceWaitIdle(*init_->GetDevice());

	//	//ImGui_ImplVulkan_DestroyFontUploadObjects();
	//}
}

//void VKImGuiManager::FeedEvent(const SDL_Event& event_)
//{
//	ImGui_ImplSDL2_ProcessEvent(&event_);
//}

void VKImGuiManager::Begin()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void VKImGuiManager::End(uint32_t index_)
{
	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCommandBuffers[index_]);
}

void VKImGuiManager::Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}
