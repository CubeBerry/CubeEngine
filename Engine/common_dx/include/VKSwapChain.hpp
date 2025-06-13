////Author: JEYOON YU
////Project: CubeEngine
////File: VKSwapChain.hpp
//#pragma once
//#include <vulkan/vulkan.hpp>
//
//const auto BUFFER_COUNT{ 2 };
//
//class VKInit;
//
//class VKSwapChain
//{
//public:
//	VKSwapChain(VKInit* init_, VkCommandPool* pool_);
//	~VKSwapChain();
//
//	VkSwapchainKHR* GetSwapChain() { return &vkSwapChain; };
//	std::array<VkFence, BUFFER_COUNT>* GetFences() { return &vkFences; };
//	std::array<std::array<VkSemaphore, 2>, BUFFER_COUNT>* GetSemaphores() { return &vkSemaphores; };
//	std::vector<VkImage>* GetSwapChainImages() { return &vkSwapChainImages; };
//	VkExtent2D* GetSwapChainImageExtent() { return &swapchainImageExtent; };
//	std::vector<VkImageView>* GetSwapChainImageViews() { return &vkSwapChainImageViews; };
//	const int* GetBufferCount() { return &BUFFER_COUNT; };
//private:
//	void InitSwapChain();
//	void InitSwapChainImage();
//	void InitSwapChainImageView();
//	void InitFence();
//	void InitSemaphore();
//
//	VKInit* vkInit;
//	VkCommandPool* vkCommandPool{ VK_NULL_HANDLE };
//
//	VkSwapchainKHR vkSwapChain{ VK_NULL_HANDLE };
//	VkExtent2D swapchainImageExtent{ 0,0 };
//	std::vector<VkImage> vkSwapChainImages;
//	std::array<VkFence, BUFFER_COUNT> vkFences{ VK_NULL_HANDLE };
//	std::array<std::array<VkSemaphore, 2>, BUFFER_COUNT> vkSemaphores{ VK_NULL_HANDLE };
//	std::vector<VkImageView> vkSwapChainImageViews;
//};