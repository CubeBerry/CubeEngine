//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKInit.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <vector>
#include <array>

class VKInit
{
public:
	VKInit();
	~VKInit();
	//void ValidationCheck();
	void InitInstance();
	void SetPhysicalDevice();
	void SetQueueFamilyIndex();
	void InitDevice();
	void InitQueue();
	void InitSurface();
	VkSurfaceFormatKHR SetSurfaceFormat();

#ifdef _DEBUG
	void PrintLayers();
	void PrintInstnaceExtensions();
	//void PrintPhysicalDevices();
	void PrintDeviceExtensions();
	void PrintPresentModes();
	void PrintMemoryProperties();
#endif
	void PrintPhysicalDevices();

	VkPhysicalDevice GetRequiredDevice(std::vector<VkPhysicalDevice>& physicalDevices, bool isDiscrete);

	VkInstance* GetInstance() { return &vkInstance; };
	VkDevice* GetDevice() { return &vkDevice; };
	VkPhysicalDevice* GetPhysicalDevice() { return &vkPhysicalDevice; };
	VkSurfaceKHR* GetSurface() { return &vkSurface; };
	uint32_t* GetQueueFamilyIndex() { return &queueFamilyIndex; };
	VkQueue* GetQueue() { return &vkQueue; };
private:
	SDL_Window* window{ nullptr };
	VkInstance vkInstance{ VK_NULL_HANDLE };
	VkPhysicalDevice vkPhysicalDevice{ VK_NULL_HANDLE };
	VkDevice vkDevice{ VK_NULL_HANDLE };
	VkQueue vkQueue{ VK_NULL_HANDLE };
	VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };

	uint32_t queueFamilyIndex{ UINT32_MAX };
};