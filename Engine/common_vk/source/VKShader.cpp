//Author: JEYOON YU
//Project: CubeEngine
//File: VKShader.cpp
#include "VKShader.hpp"
#include "VKHelper.hpp"

#include <fstream>

VKShader::VKShader(VkDevice* device_) : device(device_)
{
}

VKShader::~VKShader()
{
	//Destroy Shader Module
	vkDestroyShaderModule(*device, vertexModule, nullptr);
	vkDestroyShaderModule(*device, fragmentModule, nullptr);
	device = nullptr;
}

void VKShader::LoadShader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
	//std::filesystem::path vertexSPIRV = GLSLtoSPIRV(vertexPath);
	//std::filesystem::path fragmentSPIRV = GLSLtoSPIRV(fragmentPath);

	//vertexModule = LoadModule(vertexSPIRV);
	//fragmentModule = LoadModule(fragmentSPIRV);

	vertexModule = LoadModule(vertexPath);
	fragmentModule = LoadModule(fragmentPath);
}

VkShaderModule VKShader::LoadModule(const std::filesystem::path& spirvPath)
{
	std::ifstream shaderData(spirvPath, std::ios::in | std::ios::binary);
	if (!shaderData.is_open())
		throw std::runtime_error{ "File Does Not Exist" };

	shaderData.seekg(0, std::ios::end);
	size_t fileSize = shaderData.tellg();
	shaderData.seekg(0);
	std::vector<uint32_t> shaderCode(fileSize / sizeof(uint32_t));
	shaderData.read(reinterpret_cast<char*>(shaderCode.data()), fileSize);
	shaderData.close();

	//Create Shader Module Info
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
	createInfo.pCode = shaderCode.data();

	//Create Shader Module
	VkShaderModule shaderModule;
	VKHelper::ThrowIfFailed(vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule));

	return shaderModule;
}

const std::filesystem::path VKShader::GLSLtoSPIRV(const std::filesystem::path& glslPath)
{
	std::string command = "glslangValidator -V -o " + glslPath.string() + ".spv " + glslPath.string();
	int result = std::system(command.c_str());
	if (result == 0) return glslPath.string() + ".spv";
	throw std::runtime_error{ "Shader Creation Failed" };
}
