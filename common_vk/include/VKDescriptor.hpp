#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <array>

class VKInit;

class VKDescriptor
{
public:
	VKDescriptor(VKInit* init_);
	~VKDescriptor();

	void InitDescriptorSetLayouts();
	void InitDescriptorPool();
	void InitDescriptorSets();

	std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayout() { return &vkDescriptorSetLayouts; };
	std::array<VkDescriptorSet, 2>* GetMaterialDescriptorSets() { return &vkMaterialDescriptorSets; };
	std::array<VkDescriptorSet, 2>* GetTextureDescriptorSets() { return &vkTextureDescriptorSets; };
private:
	VKInit* vkInit;

	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
	VkDescriptorSetLayout vkMaterialDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout vkTextureDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool vkDescriptorPool{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, 2> vkMaterialDescriptorSets{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, 2> vkTextureDescriptorSets{ VK_NULL_HANDLE };
};