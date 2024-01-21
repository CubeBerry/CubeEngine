//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKRenderManager.hpp
#pragma once
#include <SDL2/SDL_vulkan.h>
#include <array>

#include "VKDescriptor.hpp"
#include "VKTexture.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"
#include "ImGuiManager.hpp"

const auto IMAGE_AVAILABLE_INDEX{ 0 };
const auto RENDERING_DONE_INDEX{ 1 };

class Window;
class VKInit;
class VKSwapChain;
class VKShader;
class VKPipeLine;
template<typename Material>
class VKUniformBuffer;

class VKRenderManager
{
public:
	VKRenderManager();
	~VKRenderManager();

	void BeginRender();
	void EndRender();
	//void EndRender(Window* window_);

	VKInit* GetVkInit() { return vkInit; }
#ifdef _DEBUG
	ImGuiManager* GetImGuiManager() { return imguiManager; };
	bool GetIsRecreated() { return isRecreated; };
#endif
private:
	void InitCommandPool();
	void InitCommandBuffer();
	void InitRenderPass();
	void InitFrameBuffer(VkExtent2D* swapchainImageExtent_, std::vector<VkImageView>* swapchainImageViews_);
	void CleanSwapChain();
	void RecreateSwapChain(Window* window_);

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
	std::array<VkSemaphore, 2> vkSemaphores;
	uint32_t frameIndex{ 0 };
	bool isRecreated{ false };

	VkCommandBuffer* currentCommandBuffer{ VK_NULL_HANDLE };
	VkFence* currentFence{ VK_NULL_HANDLE };
	VkDescriptorSet* currentVertexMaterialDescriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSet* currentTextureDescriptorSet{ VK_NULL_HANDLE };

	//--------------------Texture Render--------------------//
public:
	void LoadTexture(const std::filesystem::path& path_);
	void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_);
	void LoadLineQuad(glm::vec4 color_);

	void LoadVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);
	void LoadLineVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);

	std::vector<UniformMatrix>* GetMatrices() { return &matrices; };
	std::vector<VKTexture*>* GetTextures() { return &textures; };
private:
	std::vector<VKTexture*> textures;
	std::vector<VkDescriptorImageInfo> imageInfos;

#ifdef _DEBUG
	ImGuiManager* imguiManager;
#endif

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
};
