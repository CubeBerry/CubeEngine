//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: VKRenderManager.hpp
#pragma once
#include <SDL2/SDL_vulkan.h>
#include <array>

#include "RenderManager.hpp"
#include "VKDescriptor.hpp"
#include "VKTexture.hpp"
#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"
#include "VKImGuiManager.hpp"
#include "Material.hpp"

const auto IMAGE_AVAILABLE_INDEX{ 0 };
const auto RENDERING_DONE_INDEX{ 1 };

class Window;
class VKInit;
class VKSwapChain;
class VKShader;
class VKPipeLine;
template<typename Material>
class VKUniformBuffer;

class VKRenderManager : public RenderManager
{
public:
	VKRenderManager() { gMode = GraphicsMode::VK; };
	~VKRenderManager();
	void Initialize(SDL_Window* window_);

	void BeginRender() override;
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

#ifdef _DEBUG
	VKImGuiManager* imguiManager;
#endif

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
	void LoadTexture(const std::filesystem::path& path_, std::string name_) override;
	void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_) override;

	void DeleteWithIndex() override;

	//std::vector<VKTexture*>* GetTextures() { return &textures; };
	/*VKTexture* GetTexture(std::string name);*/
	VKTexture* GetTexture(std::string name);
private:
	std::vector<VKTexture*> textures;
	std::vector<VkDescriptorImageInfo> imageInfos;

	VKVertexBuffer<Vertex>* texVertex{ nullptr };
	VKIndexBuffer* texIndex { nullptr };

	VKUniformBuffer<VertexUniform>* uVertex{ nullptr };
	VKUniformBuffer<FragmentUniform>* uFragment{ nullptr };
};
