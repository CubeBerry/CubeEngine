//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKDescriptor.cpp
#include "VKDescriptor.hpp"
#include "VKInit.hpp"
#include "VKHelper.hpp"

VKDescriptor::VKDescriptor(VKInit* init, std::initializer_list<VKDescriptorLayout> vertexLayout, std::initializer_list<VKDescriptorLayout> fragmentLayout) : vkInit(init)
{
	InitDescriptorSetLayouts(vertexLayout, fragmentLayout);
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

void VKDescriptor::InitDescriptorSetLayouts(std::initializer_list<VKDescriptorLayout> vertexLayout, std::initializer_list<VKDescriptorLayout> fragmentLayout)
{
	std::vector<VkDescriptorSetLayoutBinding> vertexBindings;
	if (vertexLayout.size())
	{
		int count{ 0 };
		for (const auto& l : vertexLayout)
		{
			//Create Binding for Vertex Uniform Block
			VkDescriptorSetLayoutBinding binding;
			binding.binding = count;
			binding.descriptorType = static_cast<VkDescriptorType>(l.descriptorType);
			binding.descriptorCount = l.descriptorCount;
			//Only vertex shader accesses
			binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			vertexDescriptorCount += l.descriptorCount;
			vertexBindings.push_back(binding);
			count++;
		}


		//Create Descriptor Set Layout Info
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(vertexBindings.size());
		createInfo.pBindings = vertexBindings.data();

		//Create Descriptor Set Layout
		VKHelper::ThrowIfFailed(vkCreateDescriptorSetLayout(*vkInit->GetDevice(), &createInfo, nullptr, &vkVertexMaterialDescriptorSetLayout));

		vkDescriptorSetLayouts.push_back(vkVertexMaterialDescriptorSetLayout);
	}

	std::vector<VkDescriptorSetLayoutBinding> fragmentBindings;
	if (fragmentLayout.size())
	{
		int count{ 0 };
		for (const auto& l : fragmentLayout)
		{
			//Create Binding for Vertex Uniform Block
			VkDescriptorSetLayoutBinding binding;
			binding.binding = count;
			binding.descriptorType = static_cast<VkDescriptorType>(l.descriptorType);
			binding.descriptorCount = l.descriptorCount;
			//Only fragment shader accesses
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding.pImmutableSamplers = nullptr;

			switch (static_cast<VkDescriptorType>(l.descriptorType))
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				lightDescriptorCount += l.descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				fragmentDescriptorCount += l.descriptorCount;
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				samplerDescriptorCount += l.descriptorCount;
				break;
			default:
				break;
			}
			fragmentBindings.push_back(binding);
			count++;
		}


		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		VkDescriptorBindingFlags flags[] = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
		bindingFlagsInfo.pBindingFlags = flags;

		//Create Descriptor Set Layout Info
		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(fragmentBindings.size());
		createInfo.pBindings = fragmentBindings.data();
		createInfo.pNext = &bindingFlagsInfo;
		createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

		//Create Descriptor Set Layout
		VKHelper::ThrowIfFailed(vkCreateDescriptorSetLayout(*vkInit->GetDevice(), &createInfo, nullptr, &vkFragmentMaterialDescriptorSetLayout));

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
		////For Vertex
		//{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vertexDescriptorCount * 2 },
		////For Fragment
		//{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fragmentDescriptorCount * 2 },
		//{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, samplerDescriptorCount * 2 },
		////For ImGUI
		//{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		////For Texture maybe should change for batch rendering(multiple image + one sampler)
	};

	if (vertexDescriptorCount)
	{
		VkDescriptorPoolSize size;
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		size.descriptorCount = vertexDescriptorCount * 2;
		poolSize.push_back(size);
	}
	if (fragmentDescriptorCount)
	{
		VkDescriptorPoolSize size;
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		size.descriptorCount = fragmentDescriptorCount * 2;
		poolSize.push_back(size);
	}
	if (lightDescriptorCount)
	{
		VkDescriptorPoolSize size;
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		size.descriptorCount = lightDescriptorCount * 2;
		poolSize.push_back(size);
	}
	if (samplerDescriptorCount)
	{
		VkDescriptorPoolSize size;
		size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		size.descriptorCount = samplerDescriptorCount * 2;
		poolSize.push_back(size);
	}

	//For ImGui
	poolSize.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1});

	//Create DescriptorPool Info
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//For uniform buffer and combined image sampler
	createInfo.maxSets = (vertexDescriptorCount + fragmentDescriptorCount + lightDescriptorCount + samplerDescriptorCount) * 2 + 1;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	createInfo.pPoolSizes = poolSize.data();
	//ImGui requires VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	//createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	//Create DescriptorPool
	VKHelper::ThrowIfFailed(vkCreateDescriptorPool(*vkInit->GetDevice(), &createInfo, nullptr, &vkDescriptorPool));
}

void VKDescriptor::InitDescriptorSets()
{
	for (auto i = 0; i != 2; ++i)
	{
		if (vertexDescriptorCount)
		{
			//Create Vertex Material DescriptorSet Allocation Info
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = vkDescriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &vkVertexMaterialDescriptorSetLayout;

			//Allocate Material DescriptorSet
			VKHelper::ThrowIfFailed(vkAllocateDescriptorSets(*vkInit->GetDevice(), &allocateInfo, &vkVertexDescriptorSets[i]));
		}

		if (fragmentDescriptorCount || lightDescriptorCount || samplerDescriptorCount)
		{
			//Create Fragment Material DescriptorSet Allocation Info
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = vkDescriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &vkFragmentMaterialDescriptorSetLayout;

			//Allocate Material DescriptorSet
			VKHelper::ThrowIfFailed(vkAllocateDescriptorSets(*vkInit->GetDevice(), &allocateInfo, &vkFragmentDescriptorSets[i]));
		}
	}
}
