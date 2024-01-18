//Author: JEYOON YU
//Project: CubeEngine
//File: VKShader.cpp
#include "VKShader.hpp"
#include "Engine.hpp"

#include <fstream>

VKShader::VKShader() : device(Engine::Instance().GetVKInit()->GetDevice())
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
	std::filesystem::path vertexSPIRV = GLSLtoSPIRV(vertexPath);
	std::filesystem::path fragmentSPIRV = GLSLtoSPIRV(fragmentPath);

	vertexModule = LoadModule(vertexSPIRV);
	fragmentModule = LoadModule(fragmentSPIRV);
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
	try
	{
		VkShaderModule shaderModule;
		VkResult result{ VK_SUCCESS };
		result = vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			switch (result)
			{
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
				break;
			default:
				break;
			}
			std::cout << std::endl;

			throw std::runtime_error{ "Shader Module Creation Failed" };
		}
		return shaderModule;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKShader::~VKShader();
		std::exit(EXIT_FAILURE);
	}
}

const std::filesystem::path VKShader::GLSLtoSPIRV(const std::filesystem::path& glslPath)
{
	try
	{
		std::string command = "glslangValidator -V -o " + glslPath.string() + ".spv " + glslPath.string();
		int result = std::system(command.c_str());
		if (result == 0)
			return glslPath.string() + ".spv";
		else
			throw std::runtime_error{ "Shader Creation Failed" };
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKShader::~VKShader();
		std::exit(EXIT_FAILURE);
	}
}
