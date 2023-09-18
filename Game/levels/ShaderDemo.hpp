#pragma once
//#include "../ImGui/MyImGui.hpp"
#include "GameState.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "VKRenderManager.hpp"

#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"
#include "VKUniformBuffer.hpp"
#include "VKTexture.hpp"

class ShaderDemo : public GameState
{
public:
	ShaderDemo() = default;
	~ShaderDemo() {};

	void Init() override;
	void Update(float dt) override;
	void Draw(float dt) override;
	void Restart() override;
	void End() override;
private:
	VKVertexBuffer* vkVertexBuffer;
	VKIndexBuffer* vkIndexBuffer;
	struct Material
	{
		glm::vec3 color;
	} material;
	VKUniformBuffer<Material>* vkUniformBuffer;
	VKTexture* vkTexture;
	float time{ 0 };

	Window* window = nullptr;
	VKRenderManager* renderManager = nullptr;
	InputManager* inputManager = nullptr;
};