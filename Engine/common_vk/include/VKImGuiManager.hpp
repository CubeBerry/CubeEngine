//Author: JEYOON YU
//Project: CubeEngine
//File: VKImGuiManager.hpp
#pragma once
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl2.h"

#include <SDL_vulkan.h>
#include <array>
#include <vector>

class VKInit;
class VKSwapChain;
class VKDescriptor;

class VKImGuiManager
{
public:
	VKImGuiManager(
		VKInit* init_,
		SDL_Window* window_,
		VkCommandPool* cpool_,
		std::array<VkCommandBuffer, 2>* cbuffers_,
		VkDescriptorPool* dpool_,
		VkRenderPass* pass_,
		VkSampleCountFlagBits samples_
		);
	~VKImGuiManager();

	void Initialize(VKInit* init_, SDL_Window* window_, VkSampleCountFlagBits samples);
	//void FeedEvent(const SDL_Event& event_);
	void Begin();
	void End(uint32_t index_);
	void Shutdown();
	//void CleanSwapChain();
	//void RecreateSwapChain(Window* window_);

	std::array<VkCommandBuffer, 2>* GetCommandBuffers() { return &vkCommandBuffers; };
private:
	VkCommandPool vkCommandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, 2> vkCommandBuffers{ VK_NULL_HANDLE };
	VkDescriptorPool vkDescriptorPool;
	VkRenderPass renderPass{ VK_NULL_HANDLE };
};