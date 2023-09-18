#include "VKDescriptor.hpp"
#include "VKInit.hpp"
#include <iostream>

VKDescriptor::VKDescriptor(VKInit* init_) : vkInit(init_)
{
	InitDescriptorSetLayouts();
	InitDescriptorPool();
	InitDescriptorSets();
}

VKDescriptor::~VKDescriptor()
{
	//Destroy Material DescriptorSetLayout
	vkDestroyDescriptorSetLayout(*vkInit->GetDevice(), vkMaterialDescriptorSetLayout, nullptr);
	//Destroy Texture DescriptorSetLayout
	vkDestroyDescriptorSetLayout(*vkInit->GetDevice(), vkTextureDescriptorSetLayout, nullptr);
	//Destroy DescriptorPool
	vkDestroyDescriptorPool(*vkInit->GetDevice(), vkDescriptorPool, nullptr);
}

void VKDescriptor::InitDescriptorSetLayouts()
{
	{
		//Create Binding for Uniform Block
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.descriptorCount = 1;
		//Only fragment shader accesses
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Create Descriptor Set Layout Info
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = 1;
		createInfo.pBindings = &binding;

		//Create Descriptor Set Layout
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateDescriptorSetLayout(*vkInit->GetDevice(), &createInfo, nullptr, &vkMaterialDescriptorSetLayout);
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

				throw std::runtime_error{ "Material Descriptor Set Layout Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKDescriptor::~VKDescriptor();
			std::exit(EXIT_FAILURE);
		}

		vkDescriptorSetLayouts.push_back(vkMaterialDescriptorSetLayout);
	}
	
	{
		//Create Binding for Combined Image Sampler
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		//Only fragment shader accesses
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Create Descriptor Set Layout Info
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = 1;
		createInfo.pBindings = &binding;

		//Create Descriptor Set Layout
		try
		{
			VkResult result{ VK_SUCCESS };
			result = vkCreateDescriptorSetLayout(*vkInit->GetDevice(), &createInfo, nullptr, &vkTextureDescriptorSetLayout);
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

				throw std::runtime_error{ "Texture Descriptor Set Layout Creation Failed" };
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			VKDescriptor::~VKDescriptor();
			std::exit(EXIT_FAILURE);
		}

		vkDescriptorSetLayouts.push_back(vkTextureDescriptorSetLayout);
	}
}

void VKDescriptor::InitDescriptorPool()
{
	//DescriptorSet is needed to access resources from pipeline
	//DescriptorPool creates DescriptorSet

	//2nd parameter == number of uniform buffer
	std::vector<VkDescriptorPoolSize> poolSize
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 },
		//For Texture maybe should change for batch rendering(multiple image + one sampler)
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 }
	};

	//Create DescriptorPool Info
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//For uniform buffer and combined image sampler
	createInfo.maxSets = 4;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	createInfo.pPoolSizes = &poolSize[0];

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
			//Create Material DescriptorSet Allocation Info
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = vkDescriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &vkMaterialDescriptorSetLayout;

			//Allocate Material DescriptorSet
			try
			{
				VkResult result{ VK_SUCCESS };
				result = vkAllocateDescriptorSets(*vkInit->GetDevice(), &allocateInfo, &vkMaterialDescriptorSets[i]);
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
			//Create Texture DescriptorSet Allocation Info
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = vkDescriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &vkTextureDescriptorSetLayout;

			//Allocate Texture DescriptorSet
			try
			{
				VkResult result{ VK_SUCCESS };
				result = vkAllocateDescriptorSets(*vkInit->GetDevice(), &allocateInfo, &vkTextureDescriptorSets[i]);
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
