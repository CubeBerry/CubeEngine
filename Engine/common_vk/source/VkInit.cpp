//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKInit.cpp
#include "VKInit.hpp"

#include <iostream>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_syswm.h>
#include <string>
#include <sstream>
#include "Engine.hpp"

VKInit::VKInit() : window(Engine::Instance().GetWindow()->GetWindow())
{
	InitInstance();
	SetPhysicalDevice();
	SetQueueFamilyIndex();
	InitDevice();
	InitQueue();
	InitSurface();

	SetSurfaceFormat();
}

VKInit::~VKInit()
{
	//Destroy Surface
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	//Destroy Device
	vkDestroyDevice(vkDevice, nullptr);
	//Destroy Instance
	vkDestroyInstance(vkInstance, nullptr);
}

void VKInit::InitInstance()
{
	try
	{
		//Layers
		constexpr auto layerName{ "VK_LAYER_KHRONOS_validation" };

		//Display extensions
		std::vector<const char*> extensionNames
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
			//"VK_KHR_get_physical_device_properties2",
		};

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.apiVersion = VK_API_VERSION_1_3;

		//Create VkInstanceInfo
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &layerName;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
		createInfo.ppEnabledExtensionNames = &extensionNames[0];
		createInfo.pApplicationInfo = &applicationInfo;

		//Create VkInstance
		//VkResult result = VK_SUCCESS;
		if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
		{
			throw std::runtime_error{ "vkInstance Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKInit::~VKInit();
		std::exit(EXIT_FAILURE);
	}
}

void VKInit::SetPhysicalDevice()
{
	//if pPhysicalDevices == nullptr -> returns numbers of all available GPU
	uint32_t count{ 0 };
	vkEnumeratePhysicalDevices(vkInstance, &count, nullptr);

	//Get physical devices to vector
	std::vector<VkPhysicalDevice> physicalDevices{ count };
	vkEnumeratePhysicalDevices(vkInstance, &count, &physicalDevices[0]);

	//Set proper physical device
	PrintPhysicalDevices();
	std::cout << "Input GPU number: ";
	int deviceNumber{ 0 };
	std::cin >> deviceNumber;
	std::cout << std::endl;
	vkPhysicalDevice = physicalDevices[deviceNumber];
	//vkPhysicalDevice = GetRequiredDevice(physicalDevices, isDiscrete);
}

void VKInit::SetQueueFamilyIndex()
{
	//if pQueueFamilyProperties == 0 -> returns numbers of all available queues
	uint32_t count{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, 0);

	//Get Queue properties to vector
	std::vector<VkQueueFamilyProperties> queueProperties{ count };
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, &queueProperties[0]);

	for (auto i = 0; i != queueProperties.size(); ++i)
	{
		//Check available queue among queue family
		if (queueProperties[i].queueCount < 1)
			continue;

		//Check queue family provides graphics function
		if (VK_QUEUE_GRAPHICS_BIT & queueProperties[i].queueFlags)
		{
			queueFamilyIndex = i;
			break;
		}
	}
}

void VKInit::InitDevice()
{
	try
	{
		//Queue Priority
		constexpr auto priority = 1.0f;

		//Create queue info
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priority;

		//Create Physical Device Features (for polygon mode line)
		VkPhysicalDeviceFeatures deviceFeatures{};
		try
		{
			vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);
			if (deviceFeatures.fillModeNonSolid != VK_TRUE)
			{
				std::cout << "Does not support fillModeNonSolid" << std::endl;
				std::cout << std::endl;

				throw std::runtime_error{ "Device Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKInit::~VKInit();
			std::exit(EXIT_FAILURE);
		}
		//deviceFeatures.fillModeNonSolid = VK_TRUE;

		//Create 12Features info (for descriptor array)
		VkPhysicalDeviceVulkan12Features version12Features{};
		version12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		version12Features.runtimeDescriptorArray = VK_TRUE;
		version12Features.descriptorIndexing = VK_TRUE; 
		version12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		//versionFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		//versionFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
		//versionFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

		//Create Robustness2Features info (for nullDescriptor)
		VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{};
		robustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		robustness2Features.nullDescriptor = VK_TRUE;
		robustness2Features.pNext = &version12Features;

		//Create device info
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pNext = &robustness2Features;
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		//--------------------Device Extensions settings--------------------//

		std::vector<const char*> deviceExtensionNames
		{
			"VK_KHR_swapchain",
			"VK_EXT_robustness2",
			//For vkCmdSetPrimitiveTopology
			"VK_EXT_extended_dynamic_state3",
		};

		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
		deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensionNames[0];

		//--------------------Device Extensions settings end--------------------//

		//Create device
		if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS)
		{
			throw std::runtime_error{ "vkDevice Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKInit::~VKInit();
		std::exit(EXIT_FAILURE);
	}
}

void VKInit::InitQueue()
{
	//Get queue from device
	vkGetDeviceQueue(vkDevice, queueFamilyIndex, 0, &vkQueue);
}

void VKInit::InitSurface()
{
	try
	{
		//Create surface info
		//VkWin32SurfaceCreateInfoKHR surfaceInfo{};
		//surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		//surfaceInfo.hinstance = GetModuleHandle(nullptr);
		//SDL_SysWMinfo winInfo;
		//SDL_GetWindowWMInfo(window, &winInfo);
		//surfaceInfo.hwnd = static_cast<HWND>(winInfo.info.win.window);

		//Create surface
		//if (vkCreateWin32SurfaceKHR(vkInstance, &surfaceInfo, nullptr, &vkSurface) != VK_SUCCESS)
		//{
		//	throw std::runtime_error{ "vkSurface Creation Failed" };
		//}

		//Create sruface using SDL_Vulkan_CreateSurface function
		if (SDL_Vulkan_CreateSurface(window, vkInstance, &vkSurface) == SDL_FALSE)
		{
			throw std::runtime_error{ "vkSurface Creation Failed" };
		}

		//Check if physical device supports surface
		VkBool32 isSurfaceSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, queueFamilyIndex, vkSurface, &isSurfaceSupported);
		if (isSurfaceSupported != VK_TRUE)
		{
			throw std::runtime_error{ "vkSurface Does Not Supported" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKInit::~VKInit();
		std::exit(EXIT_FAILURE);
	}
}

VkSurfaceFormatKHR VKInit::SetSurfaceFormat()
{
	//if pSurfaceFormats == nullptr -> returns numbers of all available surface formats
	uint32_t count{ 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &count, nullptr);

	//Get surface formats to vector
	std::vector<VkSurfaceFormatKHR> surfaceFormats{ count };
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &count, &surfaceFormats[0]);

	return surfaceFormats[0];
}

//--------------------Initialization Debugging--------------------//
#ifdef _DEBUG

void VKInit::PrintLayers()
{
	//if pProperties == nullptr -> returns numbers of all available vulkan layers
	uint32_t count{ 0 };
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	//Get layers information to vector
	std::vector<VkLayerProperties> properties{ count };
	vkEnumerateInstanceLayerProperties(&count, &properties[0]);

	//Print layers information
	std::cout << "--------------------Layer Information--------------------" << std::endl;
	for (auto& p : properties)
	{
		std::cout << "Layer Name             : " << p.layerName << '\n'
				  << "Spec Version           : " << p.specVersion << '\n'
				  << "Implementation Version : " << p.implementationVersion << '\n'
				  << "Description            : " << p.description << '\n' << std::endl;
	}
}

void VKInit::PrintInstnaceExtensions()
{
	/*	Get extension properties from specific layer

	//if pProperties == nullptr -> returns numbers of all available extensions from a layer
	uint32_t count{ 0 };
	vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &count, nullptr);

	//Get extention properties to vector
	std::vector<VkExtensionProperties> extentionProperties{ count };
	vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &count, &extentionProperties[0]);

	*/

	//Get whole extension properties
	//if pProperties == nullptr -> returns numbers of all available extensions from a layer
	uint32_t count{ 0 };
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

	//Get extention properties to vector
	std::vector<VkExtensionProperties> properties{ count };
	vkEnumerateInstanceExtensionProperties(nullptr, &count, &properties[0]);

	std::cout << "--------------------Instance Extension Information--------------------" << std::endl;
	for (auto& p : properties)
	{
		std::cout << "Extension name : " << p.extensionName << '\n'
				  << "Spec version   : " << p.specVersion << '\n' << std::endl;
	}
}

//void VKInit::PrintPhysicalDevices()
//{
//	//if pPhysicalDevices == nullptr -> returns numbers of all available GPU
//	uint32_t count{ 0 };
//	vkEnumeratePhysicalDevices(vkInstance, &count, nullptr);
//
//	//Get physical devices to vector
//	std::vector<VkPhysicalDevice> physicalDevices{ count };
//	vkEnumeratePhysicalDevices(vkInstance, &count, &physicalDevices[0]);
//
//	//Get physical devices properties
//	std::cout << "--------------------Physical Device Information--------------------" << std::endl;
//	for (auto& device : physicalDevices)
//	{
//		VkPhysicalDeviceProperties properties;
//		vkGetPhysicalDeviceProperties(device, &properties);
//
//		std::cout << "Device Name : " << properties.deviceName << '\n' << std::endl;
//	}
//}

void VKInit::PrintDeviceExtensions()
{
	//if pProperties == nullptr -> returns numbers of all available device extention properties
	uint32_t count{ 0 };
	vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, nullptr);

	//Get device extentions properties to vector
	std::vector<VkExtensionProperties> properties{ count };
	vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, &properties[0]);

	std::cout << "--------------------Device Extention Information--------------------" << std::endl;
	for (auto& p : properties)
	{
		std::cout << "Extension name : " << p.extensionName << '\n'
				  << "Spec version   : " << p.specVersion << '\n' << std::endl;
	}
}

void VKInit::PrintPresentModes()
{
	//if pPresentModes == nullptr -> returns numbers of all available present modes
	uint32_t count{ 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &count, nullptr);

	//Get present modes to vector
	std::vector<VkPresentModeKHR> modes{ count };
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &count, &modes[0]);

	std::cout << "--------------------Present Mode Information--------------------" << std::endl;
	for (auto& m : modes)
	{
		switch (m)
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			std::cout << "VK_PRESENT_MODE_IMMEDIATE_KHR" << '\n';
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			std::cout << "VK_PRESENT_MODE_MAILBOX_KHR" << '\n';
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			std::cout << "VK_PRESENT_MODE_FIFO_KHR" << '\n';
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			std::cout << "VK_PRESENT_MODE_FIFO_RELAXED_KHR" << '\n';
			break;
		default:
			break;
		}
		std::cout << std::endl;
	}
}

std::string MemoryTypeToString(VkMemoryPropertyFlags flags)
{
	if (!flags)
		return "HOST_LOCAL";

	std::stringstream ss;
	bool delimiter{ false };

	if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		if (delimiter)
			ss << " | ";

		delimiter = true;
		ss << "DEVICE_LOCAL";
	}

	if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		if (delimiter)
			ss << " | ";

		delimiter = true;
		ss << "HOST_VISIBLE";
	}

	if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
		if (delimiter)
			ss << " | ";

		delimiter = true;
		ss << "HOST_COHERENT";
	}

	if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
	{
		if (delimiter)
			ss << " | ";

		delimiter = true;
		ss << "HOST_CACHED";
	}

	if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
	{
		if (delimiter)
			ss << " | ";

		delimiter = true;
		ss << "LAZILY_ALLOCATED";
	}

	return ss.str();
}

auto& operator<<(std::ostream& os, const VkMemoryType type)
{
	os << MemoryTypeToString(type.propertyFlags);
	return os;
}

auto& operator<<(std::ostream& os, const VkMemoryHeap heap)
{
	os << "Size: " << heap.size;
	return os;
}

void VKInit::PrintMemoryProperties()
{
	//Get PhysicalDevice Memory Properties
	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &properties);

	//DEVICE == GPU, HOST == CPU
	std::cout << "--------------------Memory Property Information--------------------" << std::endl;
	for (auto i = 0; i != VK_MAX_MEMORY_HEAPS; ++i)
	{
		auto heapIndex = properties.memoryTypes[i].heapIndex;

		std::cout << i << " Memory Type" << '\n'
			<< '\t' << properties.memoryTypes[i] << '\n'
			//Print Heap Size
			<< '\t' << properties.memoryHeaps[heapIndex] << '\n';
	}
	std::cout << std::endl;
	/*
	1. DEVICE_LOCAL: GPU(fast) but CPU cannot access. (Vertex Buffer, Index Buffer, Texture)
	2. HOST_VISIBLE: Both CPU(update often) and GPU(slow) can access. (Uniform Buffer)
	3. DEVICE_LOCAL + HOST_VISIBLE(AMD only): Both CPU(fast) and GPU(fast) can access.
											  However, CPU access speed is slower than
											  HOST_VISIBLE memory type. (Uniform Buffer)
	4. HOST VISIBLE + HOST CACHED: DRAM(System Memory). Use this memory type when reading
								   data written by GPU. (Rendered Data)
	5. COHERENT: Automatically synchronize CPU and GPU when reading memory.
	*/
}

#endif

void VKInit::PrintPhysicalDevices()
{
	//if pPhysicalDevices == nullptr -> returns numbers of all available GPU
	uint32_t count{ 0 };
	vkEnumeratePhysicalDevices(vkInstance, &count, nullptr);

	//Get physical devices to vector
	std::vector<VkPhysicalDevice> physicalDevices{ count };
	vkEnumeratePhysicalDevices(vkInstance, &count, &physicalDevices[0]);

	//Get physical devices properties
	std::cout << "--------------------Physical Device Information--------------------" << std::endl;
	for (auto& device : physicalDevices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		std::cout << "Device Name : " << properties.deviceName << '\n' << std::endl;
	}
}

VkPhysicalDevice VKInit::GetRequiredDevice(std::vector<VkPhysicalDevice>& physicalDevices, bool isDiscrete)
{
	uint32_t returnValue{ 0 };
	for (auto& device : physicalDevices)
	{
		//Get Physical Device Properties
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		//Find Device is Discrete or Integrated Graphic
		if ((properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && isDiscrete == true) ||
			(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && isDiscrete == false))
		{
			std::cout << "Set " << "Device : " << properties.deviceName << std::endl;
			return physicalDevices[returnValue];
		}
		returnValue++;
	}
	//Return First Physical Device
	return  physicalDevices[0];
}
