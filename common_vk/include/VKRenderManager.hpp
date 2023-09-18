#pragma once
//#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include <array>

#include "VKInit.hpp"
#include "VKDescriptor.hpp"
#include "VKTexture.hpp"
#include "VKSwapChain.hpp"

const auto IMAGE_AVAILABLE_INDEX{ 0 };
const auto RENDERING_DONE_INDEX{ 1 };

class Window;
//class VKInit;
//class VKSwapChain;
class VKShader;
class VKPipeLine;
template<typename Material>
class VKUniformBuffer;

class VKRenderManager
{
public:
	VKRenderManager(SDL_Window* window_);
	~VKRenderManager();
	void InitCommandPool();
	void InitCommandBuffer();
	void InitRenderPass();
	void InitFrameBuffer(VkExtent2D* swapchainImageExtent_, std::vector<VkImageView>* swapchainImageViews_);
	void CleanSwapChain();
	void RecreateSwapChain(Window* window_);

	void OldClearColor();
	//void NewClearColor(Window* window_);
	//void DrawVerticesTriangle(VkBuffer* buffer_);
	void DrawIndicesTriangle(VkBuffer* vertex_, VkBuffer* index_);

	template<typename Material>
	void BeginRender(Window* window_, VKUniformBuffer<Material>* uniform_, Material* material_, VKTexture* texture_);
	void EndRender(Window* window_);

	VkCommandPool* GetCommandPool() { return &vkCommandPool; };

	VKInit* GetVkInit() { return vkInit; }
private:
	SDL_Window* window;
	VKInit* vkInit;
	VKSwapChain* vkSwapChain;

	VkCommandPool vkCommandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, BUFFER_COUNT> vkCommandBuffers{ VK_NULL_HANDLE };
	VkRenderPass vkRenderPass{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> vkFrameBuffers;

	VKShader* vkShader;
	VKPipeLine* vkPipeline;
	VKDescriptor* vkDescriptor;

	uint32_t swapchainIndex;
	VkImage swapchainImage;
	uint32_t frameIndex{ 0 };

	VkCommandBuffer* currentCommandBuffer{ VK_NULL_HANDLE };
	VkFence* currentFence{ VK_NULL_HANDLE };
	VkDescriptorSet* currentMaterialDescriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSet* currentTextureDescriptorSet{ VK_NULL_HANDLE };
};

template<typename Material>
inline void VKRenderManager::BeginRender(Window* window_, VKUniformBuffer<Material>* uniform_, Material* material_, VKTexture* texture_)
{
	auto& vkSemaphore = (*vkSwapChain->GetSemaphores())[frameIndex];

	//Get image index from swapchain
	//uint32_t swapchainIndex;
	VkResult result = vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, (*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		RecreateSwapChain(window_);
	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	//	throw std::runtime_error("Failed Acquring SwapChain Image");

	//Get swapchain image
	//VkImage swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
	swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];

	currentFence = &(*vkSwapChain->GetFences())[frameIndex];

	//Wait for fence to be signaled
	if (vkGetFenceStatus(*vkInit->GetDevice(), *currentFence) == VK_NOT_READY)
		vkWaitForFences(*vkInit->GetDevice(), 1, currentFence, VK_TRUE, UINT64_MAX);

	//Set fence to unsignaled
	vkResetFences(*vkInit->GetDevice(), 1, currentFence);

	//--------------------Descriptor Update--------------------//

	auto& vkUniformBuffer = (*uniform_->GetUniformBuffers())[frameIndex];
	 
	currentMaterialDescriptorSet = &(*vkDescriptor->GetMaterialDescriptorSets())[frameIndex];
	{
		//Create Material DescriptorBuffer Info
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vkUniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Material);

		//Define which resource descriptor set will point
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = *currentMaterialDescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.pBufferInfo = &bufferInfo;

		//Update DescriptorSet
		//DescriptorSet does not have to update every frame since it points same uniform buffer
		vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	currentTextureDescriptorSet = &(*vkDescriptor->GetTextureDescriptorSets())[frameIndex];
	{
		//Create Texture DescriptorBuffer Info
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = *texture_->GetSampler();
		imageInfo.imageView = *texture_->GetImageView();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//Define which resource descriptor set will point
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = *currentTextureDescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pImageInfo = &imageInfo;

		//Update DescriptorSet
		//DescriptorSet does not have to update every frame since it points same uniform buffer
		vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	//Update Uniform Material
	uniform_->UpdateUniform(material_, frameIndex);

	//--------------------Descriptor Update End--------------------//

	currentCommandBuffer = &vkCommandBuffers[frameIndex];

	//Reset command buffer
	vkResetCommandBuffer(*currentCommandBuffer, 0);

	//Create command buffer begin info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin command buffer
	vkBeginCommandBuffer(*currentCommandBuffer, &beginInfo);

	//Change image layout to TRANSFER_DST_OPTIMAL
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
		barrier.image = swapchainImage;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		//Record pipeline barrier for chainging image layout
		vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	//Set clear color
	VkClearValue clearValue{};
	clearValue.color.float32[0] = 1.0f;	//R
	clearValue.color.float32[1] = 0.0f;	//G
	clearValue.color.float32[2] = 1.0f;	//B
	clearValue.color.float32[3] = 1.0f;	//A

	//Create renderpass begin info
	VkRenderPassBeginInfo renderpassBeginInfo{};
	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = vkRenderPass;
	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = &clearValue;

	//Begin renderpass
	vkCmdBeginRenderPass(*currentCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}