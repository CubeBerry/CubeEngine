//Author: JEYOON YU
//Project: CubeEngine
//File: VKDescriptor.hpp
#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <array>

class VKInit;

class VKDescriptor
{
public:
	VKDescriptor(VKInit* init);
	~VKDescriptor();

	void InitDescriptorSetLayouts();
	void InitDescriptorPool();
	void InitDescriptorSets();

	VkDescriptorPool* GetDescriptorPool() { return &vkDescriptorPool; };
	std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayout() { return &vkDescriptorSetLayouts; };
	std::array<VkDescriptorSet, 2>* GetVertexMaterialDescriptorSets() { return &vkVertexMaterialDescriptorSets; };
	std::array<VkDescriptorSet, 2>* GetFragmentMaterialDescriptorSets() { return &vkFragmentMaterialDescriptorSets; };
	//std::array<VkDescriptorSet, 2>* GetTextureDescriptorSets() { return &vkTextureDescriptorSets; };
private:
	VKInit* vkInit;

	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
	VkDescriptorSetLayout vkVertexMaterialDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout vkFragmentMaterialDescriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool vkDescriptorPool{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, 2> vkVertexMaterialDescriptorSets{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, 2> vkFragmentMaterialDescriptorSets{ VK_NULL_HANDLE };
};