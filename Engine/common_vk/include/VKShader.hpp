//Author: JEYOON YU
//Project: CubeEngine
//File: VKShader.hpp
#pragma once
#include <iostream>
#include <filesystem>
#include <vulkan/vulkan.hpp>

class VKShader
{
public:
	VKShader(VkDevice* device_);
	~VKShader();
	void LoadShader(const std::filesystem::path& vertexPath,
		const std::filesystem::path& fragmentPath);

	VkShaderModule* GetVertexModule() { return &vertexModule; };
	VkShaderModule* GetFragmentModule() { return &fragmentModule; };
private:
	VkDevice* device{ nullptr };
	VkShaderModule LoadModule(const std::filesystem::path& spirvPath);
	const std::filesystem::path GLSLtoSPIRV(const std::filesystem::path& glslPath);

	VkShaderModule vertexModule{ VK_NULL_HANDLE };
	VkShaderModule fragmentModule{ VK_NULL_HANDLE };
};