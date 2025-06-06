//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKRenderManager.hpp
#pragma once
#include <array>

#include "RenderManager.hpp"
#include "VKDescriptor.hpp"
#include "VKTexture.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"
#include "VKImGuiManager.hpp"

#include <SDL_vulkan.h>

constexpr auto IMAGE_AVAILABLE_INDEX{ 0 };
constexpr auto RENDERING_DONE_INDEX{ 1 };

#define MAX_OBJECT_SIZE 500
#define MAX_LIGHT_SIZE 10

class Window;
class VKInit;
class VKSwapChain;
class VKShader;
class VKPipeLine;
template<typename Material>
class VKUniformBuffer;
class VKSkybox;

class VKRenderManager : public RenderManager
{
public:
	VKRenderManager() { gMode = GraphicsMode::VK; };
	~VKRenderManager();
	void Initialize(SDL_Window* window_);

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;
	//void EndRender(Window* window_);
private:
	void InitCommandPool();
	void InitCommandBuffer();
	void InitRenderPass();
	void InitFrameBuffer(VkExtent2D* swapchainImageExtent_, std::vector<VkImageView>* swapchainImageViews_);
	void CleanSwapChain();
	void RecreateSwapChain();

	SDL_Window* window;
	VKInit* vkInit;
	VKSwapChain* vkSwapChain;

	VkCommandPool vkCommandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, 2> vkCommandBuffers{ VK_NULL_HANDLE };
	VkRenderPass vkRenderPass{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> vkFrameBuffers;

	//Depth Buffering
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkFormat depthFormat;
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(*vkInit->GetPhysicalDevice(), format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("Failed To Find Supported Format!");
	};
	VkFormat FindDepthFormat()
	{
		return FindSupportedFormat
		(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
	bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	};
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements requirements_, VkMemoryPropertyFlags properties_)
	{
		//Get Physical Device Memory Properties
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(*vkInit->GetPhysicalDevice(), &physicalDeviceMemoryProperties);

		//Find memory type index which satisfies both requirement and property
		for (uint32_t i = 0; i != physicalDeviceMemoryProperties.memoryTypeCount; ++i)
		{
			//Check if memory is allocatable at ith memory type
			if (!(requirements_.memoryTypeBits & (1 << i)))
				continue;

			//Check if satisfies memory property
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties_) != properties_)
				continue;

			return i;
		}
		return UINT32_MAX;
	}
	void CreateDepthBuffer();

	//MSAA
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	VkFormat imageFormat;
	VkSampleCountFlagBits msaaSamples{ VK_SAMPLE_COUNT_1_BIT };
	//This function should be called after VKInit's Initialize() function
	VkSampleCountFlagBits GetMaxUsableSampleCount() const
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(*vkInit->GetPhysicalDevice(), &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; };
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; };
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; };
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; };
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; };
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; };
		return VK_SAMPLE_COUNT_1_BIT;
	}
	void CreateColorResources();

	VKImGuiManager* imguiManager;

	VKShader* vkShader2D;
	VKPipeLine* vkPipeline2D;
	VKShader* vkShader3D;
	VKPipeLine* vkPipeline3D;
	VKPipeLine* vkPipeline3DLine;
	VKDescriptor* vkDescriptor;

	uint32_t swapchainIndex;
	VkImage swapchainImage;
	std::array<VkSemaphore, 2> vkSemaphores;
	uint32_t frameIndex{ 0 };
	bool isRecreated{ false };

	VkCommandBuffer* currentCommandBuffer{ VK_NULL_HANDLE };
	VkFence* currentFence{ VK_NULL_HANDLE };
	VkDescriptorSet* currentVertexDescriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSet* currentFragmentDescriptorSet{ VK_NULL_HANDLE };
public:
	//--------------------Common--------------------//
	void DeleteWithIndex(int id) override;
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;
	VKVertexBuffer<Vertex>* AllocateVertexBuffer(std::vector<Vertex>& vertices) const
	{
		return new VKVertexBuffer<Vertex>(vkInit, &vertices);
	}
#ifdef _DEBUG
	VKVertexBuffer<ThreeDimension::NormalVertex>* AllocateNormalVertexBuffer(std::vector<ThreeDimension::NormalVertex>& vertices) const
	{
		return new VKVertexBuffer<ThreeDimension::NormalVertex>(vkInit, &vertices);
	}
#endif
	VKIndexBuffer* AllocateIndexBuffer(std::vector<uint32_t>& indices)
	{
		return new VKIndexBuffer(vkInit, &vkCommandPool, &indices);
	}
	[[nodiscard]] VKUniformBuffer<VertexUniform>* AllocateVertexUniformBuffer() const
	{
		return new VKUniformBuffer<VertexUniform>(vkInit, 1);
	}
	[[nodiscard]] VKUniformBuffer<FragmentUniform>* AllocateFragmentUniformBuffer() const
	{
		return new VKUniformBuffer<FragmentUniform>(vkInit, 1);
	}
	[[nodiscard]] VKUniformBuffer<ThreeDimension::Material>* AllocateMaterialUniformBuffer() const
	{
		return new VKUniformBuffer<ThreeDimension::Material>(vkInit, 1);
	}

	VKTexture* GetTexture(std::string name);
	std::vector<VKTexture*> GetTextures() { return textures; }

	//--------------------3D Render--------------------//
	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
private:
	//--------------------Common--------------------//
	VkSampler immutableSampler;
	std::vector<VKTexture*> textures;
	std::vector<VkDescriptorImageInfo> imageInfos;

#ifdef _DEBUG
	VKShader* vkNormal3DShader;
	VKPipeLine* vkPipeline3DNormal;
#endif

	//Lighting
	VKUniformBuffer<ThreeDimension::DirectionalLightUniform>* directionalLightUniformBuffer{ nullptr };
	VKUniformBuffer<ThreeDimension::PointLightUniform>* pointLightUniformBuffer{ nullptr };
	//int activeLights[2] = { 0, 0 };
	struct alignas(16) PushConstants
	{
		int activeDirectionalLight;
		int activePointLight;
	} pushConstants;

	//Skyobx
	VKSkybox* skybox;
	VKShader* skyboxShader;
	VKPipeLine* vkPipeline3DSkybox;
	VKDescriptor* skyboxDescriptor;
	VKVertexBuffer<glm::vec3>* skyboxVertexBuffer{ nullptr };
	VkDescriptorSet* currentVertexSkyboxDescriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSet* currentFragmentSkyboxDescriptorSet{ VK_NULL_HANDLE };
};
