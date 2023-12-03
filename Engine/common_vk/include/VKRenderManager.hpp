#pragma once
//#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include <array>

#include "VKInit.hpp"
#include "VKDescriptor.hpp"
#include "VKTexture.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"
#include "VKSwapChain.hpp"
#include "ImGuiManager.hpp"

struct TestCamera
{
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 0.0f };
	glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f };
	glm::vec3 upVector{ 0.0f, 1.0f, 0.0f };

	float fov = 45.0f;
	float aspectRatio = 640 / 480; // Replace screenWidth and screenHeight with actual values
	float nearClip = 0.1f;
	float farClip = 100.0f;
};

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
	VKRenderManager(SDL_Window* window_, bool isDiscrete);
	~VKRenderManager();

	void Render();
	//void EndRender(Window* window_);

	VkCommandPool* GetCommandPool() { return &vkCommandPool; };
	VKInit* GetVkInit() { return vkInit; }
	ImGuiManager* GetImGuiManager() { return imguiManager; };

	//--------------------Texture Render--------------------//

	void LoadTexture(const std::filesystem::path& path_);
	void LoadQuad(glm::vec4 color_, float isTex_);
	void LoadLineQuad(glm::vec4 color_);

	void LoadVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);
	void LoadLineVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);
	std::vector<UniformMatrix>* GetMatrices() { return &matrices; };
private:
	void InitCommandPool();
	void InitCommandBuffer();
	void InitRenderPass();
	void InitFrameBuffer(VkExtent2D* swapchainImageExtent_, std::vector<VkImageView>* swapchainImageViews_);
	void CleanSwapChain();
	void RecreateSwapChain(Window* window_);

	//void OldClearColor();
	//void NewClearColor(Window* window_);
	//void DrawVerticesTriangle(VkBuffer* buffer_);
	//void DrawIndicesTriangle(VkBuffer* vertex_, VkBuffer* index_);
private:
	SDL_Window* window;
	VKInit* vkInit;
	VKSwapChain* vkSwapChain;

	VkCommandPool vkCommandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, 2> vkCommandBuffers{ VK_NULL_HANDLE };
	VkRenderPass vkRenderPass{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> vkFrameBuffers;

	VKShader* vkTextureShader;
	VKShader* vkLineShader;
	VKPipeLine* vkTexurePipeline;
	VKPipeLine* vkLinePipeline;
	VKDescriptor* vkDescriptor;

	uint32_t swapchainIndex;
	VkImage swapchainImage;
	uint32_t frameIndex{ 0 };

	VkCommandBuffer* currentCommandBuffer{ VK_NULL_HANDLE };
	VkFence* currentFence{ VK_NULL_HANDLE };
	VkDescriptorSet* currentVertexMaterialDescriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSet* currentTextureDescriptorSet{ VK_NULL_HANDLE };

	//--------------------Texture Render--------------------//

	std::vector<VKTexture*> textures;
	std::vector<VkDescriptorImageInfo> imageInfos;

	ImGuiManager* imguiManager;

	std::vector<Vertex> texVertices;
	VKVertexBuffer* texVertex{ nullptr };
	std::vector<uint16_t> texIndices;
	VKIndexBuffer* texIndex{ nullptr };

	std::vector<Vertex> lineVertices;
	VKVertexBuffer* lineVertex{ nullptr };
	std::vector<uint16_t> lineIndices;
	VKIndexBuffer* lineIndex{ nullptr };

	std::vector<UniformMatrix> matrices;
	VKUniformBuffer<UniformMatrix>* uniform{ nullptr };
	unsigned int quadCount{ 0 };

	//Variable for UniformMatrix
	TestCamera tesCamera;
};

//void VKRenderManager::Render(Window* window_)
//{
//	auto& vkSemaphore = (*vkSwapChain->GetSemaphores())[frameIndex];
//
//	//Get image index from swapchain
//	//uint32_t swapchainIndex;
//	VkResult result = vkAcquireNextImageKHR(*vkInit->GetDevice(), *vkSwapChain->GetSwapChain(), UINT64_MAX, (*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX], VK_NULL_HANDLE, &swapchainIndex);
//	if (result == VK_ERROR_OUT_OF_DATE_KHR)
//		RecreateSwapChain(window_);
//	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	//	throw std::runtime_error("Failed Acquring SwapChain Image");
//
//	//Get swapchain image
//	//VkImage swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
//	swapchainImage = (*vkSwapChain->GetSwapChainImages())[swapchainIndex];
//
//	currentFence = &(*vkSwapChain->GetFences())[frameIndex];
//
//	//Wait for fence to be signaled
//	if (vkGetFenceStatus(*vkInit->GetDevice(), *currentFence) == VK_NOT_READY)
//		vkWaitForFences(*vkInit->GetDevice(), 1, currentFence, VK_TRUE, UINT64_MAX);
//
//	//Set fence to unsignaled
//	vkResetFences(*vkInit->GetDevice(), 1, currentFence);
//
//	//--------------------Descriptor Update--------------------//
//
//	auto& vkUniformBuffer = (*textures[0].GetUniformBuffers())[frameIndex];
//	currentMaterialDescriptorSet = &(*vkDescriptor->GetMaterialDescriptorSets())[frameIndex];
//	{
//		//Create Vertex Material DescriptorBuffer Info
//		VkDescriptorBufferInfo bufferInfo{};
//		bufferInfo.buffer = vkUniformBuffer;
//		bufferInfo.offset = 0;
//		bufferInfo.range = sizeof(glm::mat3);
//
//		//Define which resource descriptor set will point
//		VkWriteDescriptorSet descriptorWrite{};
//		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//		descriptorWrite.dstSet = *currentMaterialDescriptorSet;
//		descriptorWrite.dstBinding = 0;
//		descriptorWrite.descriptorCount = 1;
//		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//		descriptorWrite.pBufferInfo = &bufferInfo;
//
//		//Update DescriptorSet
//		//DescriptorSet does not have to update every frame since it points same uniform buffer
//		vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
//	}
//
//	//auto& vkUniformBuffer = (*uniform_->GetUniformBuffers())[frameIndex];
//	//currentMaterialDescriptorSet = &(*vkDescriptor->GetMaterialDescriptorSets())[frameIndex];
//	//{
//	//	//Create Fragment Material DescriptorBuffer Info
//	//	VkDescriptorBufferInfo bufferInfo{};
//	//	bufferInfo.buffer = vkUniformBuffer;
//	//	bufferInfo.offset = 0;
//	//	bufferInfo.range = sizeof(Material);
//
//	//	//Define which resource descriptor set will point
//	//	VkWriteDescriptorSet descriptorWrite{};
//	//	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//	//	descriptorWrite.dstSet = *currentMaterialDescriptorSet;
//	//	descriptorWrite.dstBinding = 0;
//	//	descriptorWrite.descriptorCount = 1;
//	//	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	//	descriptorWrite.pBufferInfo = &bufferInfo;
//
//	//	//Update DescriptorSet
//	//	//DescriptorSet does not have to update every frame since it points same uniform buffer
//	//	vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
//	//}
//
//	currentTextureDescriptorSet = &(*vkDescriptor->GetTextureDescriptorSets())[frameIndex];
//	{
//		//Create Texture DescriptorBuffer Info
//		VkDescriptorImageInfo imageInfo{};
//		imageInfo.sampler = *textures[0].GetSampler();
//		imageInfo.imageView = *textures[0].GetImageView();
//		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//
//		//Define which resource descriptor set will point
//		VkWriteDescriptorSet descriptorWrite{};
//		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//		descriptorWrite.dstSet = *currentTextureDescriptorSet;
//		descriptorWrite.dstBinding = 0;
//		descriptorWrite.descriptorCount = 1;
//		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//		descriptorWrite.pImageInfo = &imageInfo;
//
//		//Update DescriptorSet
//		//DescriptorSet does not have to update every frame since it points same uniform buffer
//		vkUpdateDescriptorSets(*vkInit->GetDevice(), 1, &descriptorWrite, 0, nullptr);
//	}
//
//	//Update Uniform Material
//	glm::mat3 mat(1);
//	//Includes Updating Uniform Function
//	textures[0].Resize(mat, frameIndex);
//	//uniform_->UpdateUniform(mat, frameIndex);
//
//	//--------------------Descriptor Update End--------------------//
//
//	currentCommandBuffer = &vkCommandBuffers[frameIndex];
//
//	//Reset command buffer
//	vkResetCommandBuffer(*currentCommandBuffer, 0);
//
//	//Create command buffer begin info
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	//Begin command buffer
//	vkBeginCommandBuffer(*currentCommandBuffer, &beginInfo);
//
//	//Change image layout to TRANSFER_DST_OPTIMAL
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = 0;
//		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//Set clear color
//	VkClearValue clearValue{};
//	clearValue.color.float32[0] = 1.0f;	//R
//	clearValue.color.float32[1] = 0.0f;	//G
//	clearValue.color.float32[2] = 1.0f;	//B
//	clearValue.color.float32[3] = 1.0f;	//A
//
//	//Create renderpass begin info
//	VkRenderPassBeginInfo renderpassBeginInfo{};
//	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderpassBeginInfo.renderPass = vkRenderPass;
//	renderpassBeginInfo.framebuffer = vkFrameBuffers[swapchainIndex];
//	renderpassBeginInfo.renderArea.extent = *vkSwapChain->GetSwapChainImageExtent();
//	renderpassBeginInfo.clearValueCount = 1;
//	renderpassBeginInfo.pClearValues = &clearValue;
//
//	//Begin renderpass
//	vkCmdBeginRenderPass(*currentCommandBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	//--------------------Begin Draw--------------------//
//
//	//Draw Quad
//	VkDeviceSize vertexBufferOffset{ 0 };
//	//Bind Vertex Buffer
//	vkCmdBindVertexBuffers(*currentCommandBuffer, 0, 1, textures[0].GetVertexBuffer(), &vertexBufferOffset);
//	//Bind Index Buffer
//	vkCmdBindIndexBuffer(*currentCommandBuffer, *textures[0].GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
//	//Bind Pipeline
//	vkCmdBindPipeline(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLine());
//	//Bind Material DescriptorSet
//	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLineLayout(), 0, 1, currentMaterialDescriptorSet, 0, nullptr);
//	//Bind Texture DescriptorSet
//	vkCmdBindDescriptorSets(*currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *vkPipeline->GetPipeLineLayout(), 1, 1, currentTextureDescriptorSet, 0, nullptr);
//	//Draw
//	vkCmdDrawIndexed(*currentCommandBuffer, 4, 1, 0, 0, 0);
//
//	//--------------------End Draw--------------------//
//
//		//End renderpass
//	vkCmdEndRenderPass(*currentCommandBuffer);
//
//	//Change image layout to PRESENT_SRC_KHR
//	{
//		VkImageMemoryBarrier barrier{};
//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		barrier.dstAccessMask = 0;
//		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		barrier.srcQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.dstQueueFamilyIndex = *vkInit->GetQueueFamilyIndex();
//		barrier.image = swapchainImage;
//		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		barrier.subresourceRange.levelCount = 1;
//		barrier.subresourceRange.layerCount = 1;
//
//		//Record pipeline barrier for chainging image layout
//		vkCmdPipelineBarrier(*currentCommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//	}
//
//	//End command buffer
//	vkEndCommandBuffer(*currentCommandBuffer);
//
//	//Create submit info
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	//Wait until swapchain image is ready after calculating pixel's result
//	//Define pipeline stage that semaphore must be signaled
//	constexpr VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[IMAGE_AVAILABLE_INDEX];
//	submitInfo.pWaitDstStageMask = &waitDstStageMask;
//
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = currentCommandBuffer;
//
//	//Define semaphore that informs when command buffer is processed
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	//Submit queue to command buffer
//	vkQueueSubmit(*vkInit->GetQueue(), 1, &submitInfo, *currentFence);
//
//	//Wait until all submitted command buffers are handled
//	vkDeviceWaitIdle(*vkInit->GetDevice());
//
//	//Create present info
//	VkPresentInfoKHR presentInfo{};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//	//Define semaphore that waits to ensure command buffer to be processed
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = &(*vkSwapChain->GetSemaphores())[RENDERING_DONE_INDEX];
//
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = vkSwapChain->GetSwapChain();
//	presentInfo.pImageIndices = &swapchainIndex;
//
//	//Render image on screen
//	VkResult result2 = vkQueuePresentKHR(*vkInit->GetQueue(), &presentInfo);
//	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR)
//		RecreateSwapChain(window_);
//	//else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	//	throw std::runtime_error("Failed Acquring SwapChain Image");
//
//	frameIndex = ++frameIndex % *vkSwapChain->GetBufferCount();
//}