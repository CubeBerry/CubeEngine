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

class VerticesDemo : public GameState
{
public:
	VerticesDemo() = default;
	~VerticesDemo() {};

	void Init() override;
	void Update(float dt) override;
	void Draw(float dt) override;
	void Restart() override;
	void End() override;

private:

	struct Material
	{
		glm::vec3 color{ 1.f };
	} material;

	struct Mesh
	{
		VKVertexBuffer* vkVertexBuffer;
		VKIndexBuffer* vkIndexBuffer;
	};

	VKTexture* vkTexture;
	VKUniformBuffer<Material>* vkUniformBuffer;
	std::vector<Mesh> meshs;

	Window* window = nullptr;
	VKRenderManager* renderManager = nullptr;
	InputManager* inputManager = nullptr;
};