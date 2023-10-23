#pragma once
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl2.h"

#include <SDL2/SDL_vulkan.h>
#include <array>
#include <vector>

class VKInit;
class VKSwapChain;
class VKDescriptor;
class Window;

class ImGuiManager
{
public:
	ImGuiManager(VKInit* init_, VKSwapChain* chain_, VkPipeline* pipeline_, SDL_Window* window_, std::array<VkCommandBuffer, 2> buffers_);
	~ImGuiManager();

	void Initialize(VkDescriptorPool* pool_, VkRenderPass* pass_, VkCommandPool cpool_);
	void FeedEvent(const SDL_Event& event_);
	void Begin();
	void End(uint32_t index_);
	void Shutdown();
	//void CleanSwapChain();
	//void RecreateSwapChain(Window* window_);

	std::array<VkCommandBuffer, 2>* GetCommandBuffers() { return &vkCommandBuffers; };
private:
	VKInit* vkInit;
	VKSwapChain* vkSwapChain;
	SDL_Window* window;
	VkPipeline* pipeline;

	VkCommandPool vkCommandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, 2> vkCommandBuffers{ VK_NULL_HANDLE };
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	VkDescriptorPool vkDescriptorPool;
	std::vector<VkFramebuffer> vkFrameBuffers;

	VkImage swapchainImage;
	VkFence* currentFence;
};