//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKInit.hpp
#pragma once
#include <vulkan/vulkan.hpp>
#include <SDL.h>
#include <vector>
#include <array>

class VKInit
{
public:
	VKInit() = default;
	~VKInit();
	void Initialize(SDL_Window* window);

	//void ValidationCheck();
	void InitInstance();
	void SetPhysicalDevice();
	void SetQueueFamilyIndex();
	void InitDevice();
	void InitQueue();
	void InitSurface(SDL_Window* window);
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

	VkInstance* GetInstance() { return &vkInstance; }
	VkDevice* GetDevice() { return &vkDevice; }
	VkPhysicalDevice* GetPhysicalDevice() { return &vkPhysicalDevice; }
	VkSurfaceKHR* GetSurface() { return &vkSurface; }
	uint32_t* GetQueueFamilyIndex() { return &queueFamilyIndex; }
	VkQueue* GetQueue() { return &vkQueue; }

	VkDeviceSize GetMinUniformBufferOffsetAlignment() const
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);
		return deviceProperties.limits.minUniformBufferOffsetAlignment;
	}
private:
	VkInstance vkInstance{ VK_NULL_HANDLE };
	VkPhysicalDevice vkPhysicalDevice{ VK_NULL_HANDLE };
	VkDevice vkDevice{ VK_NULL_HANDLE };
	VkQueue vkQueue{ VK_NULL_HANDLE };
	VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };

	uint32_t queueFamilyIndex{ UINT32_MAX };
};