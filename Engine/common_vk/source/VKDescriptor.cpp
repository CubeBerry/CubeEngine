//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKDescriptor.cpp
#include "VKDescriptor.hpp"
#include "VKInit.hpp"
#include <iostream>

VKDescriptor::VKDescriptor(VKInit* init) : vkInit(init)
{
	InitDescriptorSetLayouts();
	InitDescriptorPool();
	InitDescriptorSets();
}

VKDescriptor::~VKDescriptor()
{
	//Destroy Vertex Material DescriptorSetLayout
	vkDestroyDescriptorSetLayout(*vkInit->GetDevice(), vkVertexMaterialDescriptorSetLayout, nullptr);
	//Destroy Fragment Material DescriptorSetLayout
	vkDestroyDescriptorSetLayout(*vkInit->GetDevice(), vkFragmentMaterialDescriptorSetLayout, nullptr);
	//Destroy Texture DescriptorSetLayout
	//vkDestroyDescriptorSetLayout(*vkInit->GetDevice(), vkTextureDescriptorSetLayout, nullptr);
	//Destroy DescriptorPool
	vkDestroyDescriptorPool(*vkInit->GetDevice(), vkDescriptorPool, nullptr);
}

void VKDescriptor::InitDescriptorSetLayouts()
{
	{
		//Create Binding for Vertex Uniform Block
		VkDescriptorSetLayoutBinding binding[2];
		binding[0].binding = 0;
		binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding[0].descriptorCount = 1;
		//Only vertex shader accesses
		binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		//Create Binding for Lighting Uniform Block
		binding[1].binding = 1;
		binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding[1].descriptorCount = 1;
		//Only vertex shader accesses
		binding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		//Create Descriptor Set Layout Info
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = 2;
		createInfo.pBindings = binding;

		//Create Descriptor Set Layout
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateDescriptorSetLayout(*vkInit->GetDevice(), &createInfo, nullptr, &vkVertexMaterialDescriptorSetLayout);
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

				throw std::runtime_error{ "Vertex Material Descriptor Set Layout Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKDescriptor::~VKDescriptor();
			std::exit(EXIT_FAILURE);
		}

		vkDescriptorSetLayouts.push_back(vkVertexMaterialDescriptorSetLayout);
	}

	{
		//Create Binding for Fragment Uniform Block
		std::array< VkDescriptorSetLayoutBinding, 3> bindings;
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		//Only fragment shader accesses
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		//Create Binding for Combined Image Sampler
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[1].descriptorCount = 500;
		//Only fragment shader accesses
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].pImmutableSamplers = nullptr;

		//Create Binding for Material
		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[2].descriptorCount = 1;
		//Only fragment shader accesses
		bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[2].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		VkDescriptorBindingFlags flags[] = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
		bindingFlagsInfo.pBindingFlags = flags;

		//Create Descriptor Set Layout Info
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();
		createInfo.pNext = &bindingFlagsInfo;
		createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

		//Create Descriptor Set Layout
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateDescriptorSetLayout(*vkInit->GetDevice(), &createInfo, nullptr, &vkFragmentMaterialDescriptorSetLayout);
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

				throw std::runtime_error{ "Fragment Material Descriptor Set Layout Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKDescriptor::~VKDescriptor();
			std::exit(EXIT_FAILURE);
		}

		vkDescriptorSetLayouts.push_back(vkFragmentMaterialDescriptorSetLayout);
	}
}

void VKDescriptor::InitDescriptorPool()
{
	//DescriptorSet is needed to access resources from pipeline
	//DescriptorPool creates DescriptorSet

	//2nd parameter == number of uniform buffer
	std::vector<VkDescriptorPoolSize> poolSize
	{
		//For Vertex
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4 },
		//For Fragment
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		//For ImGUI
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		//For Texture maybe should change for batch rendering(multiple image + one sampler)
	};

	//Create DescriptorPool Info
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//For uniform buffer and combined image sampler
	createInfo.maxSets = 1009;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	createInfo.pPoolSizes = poolSize.data();
	//ImGui requires VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	//createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	//Create DescriptorPool
	try
	{
		VkResult result{ VK_SUCCESS };
		result = vkCreateDescriptorPool(*vkInit->GetDevice(), &createInfo, nullptr, &vkDescriptorPool);
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

			throw std::runtime_error{ "Descriptor Pool Creation Failed" };
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		VKDescriptor::~VKDescriptor();
		std::exit(EXIT_FAILURE);
	}
}

void VKDescriptor::InitDescriptorSets()
{
	for (auto i = 0; i != 2; ++i)
	{
		{
			//Create Vertex Material DescriptorSet Allocation Info
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = vkDescriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &vkVertexMaterialDescriptorSetLayout;

			//Allocate Material DescriptorSet
			try
			{
				VkResult result{ VK_SUCCESS };
				result = vkAllocateDescriptorSets(*vkInit->GetDevice(), &allocateInfo, &vkVertexMaterialDescriptorSets[i]);
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
					case VK_ERROR_OUT_OF_POOL_MEMORY:
						std::cout << "VK_ERROR_OUT_OF_POOL_MEMORY" << std::endl;
						break;
					default:
						break;
					}
					std::cout << std::endl;

					throw std::runtime_error{ "Descriptor Set Creation Failed" };
				}
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				VKDescriptor::~VKDescriptor();
				std::exit(EXIT_FAILURE);
			}
		}

		{
			//Create Fragment Material DescriptorSet Allocation Info
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = vkDescriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &vkFragmentMaterialDescriptorSetLayout;

			//Allocate Material DescriptorSet
			try
			{
				VkResult result{ VK_SUCCESS };
				result = vkAllocateDescriptorSets(*vkInit->GetDevice(), &allocateInfo, &vkFragmentMaterialDescriptorSets[i]);
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
					case VK_ERROR_OUT_OF_POOL_MEMORY:
						std::cout << "VK_ERROR_OUT_OF_POOL_MEMORY" << std::endl;
						break;
					default:
						break;
					}
					std::cout << std::endl;

					throw std::runtime_error{ "Descriptor Set Creation Failed" };
				}
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				VKDescriptor::~VKDescriptor();
				std::exit(EXIT_FAILURE);
			}
		}
	}
}
