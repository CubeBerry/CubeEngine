#include "Texture.hpp"
#include "Engine.hpp"

#include "Vertex.hpp"

Texture::Texture(const std::filesystem::path& path_)
{
	auto* renderManager = Engine::Engine().GetVKRenderManager();
	texture = new VKTexture(renderManager->GetVkInit(), renderManager->GetCommandPool());
	texture->LoadTexture(path_);

	std::vector<Vertex> vertices
	{
		Vertex(glm::vec3(-1.f, 1.f, 0), glm::vec3(0, 0, 0), glm::vec2(0, 0)),
		Vertex(glm::vec3(-1.f, -1.f, 0), glm::vec3(0, 0, 0), glm::vec2(0, 0)),
		Vertex(glm::vec3(1.f, 1.f, 0), glm::vec3(0, 0, 0), glm::vec2(0, 0)),
		Vertex(glm::vec3(1.f, -1.f, 0), glm::vec3(0, 0, 0), glm::vec2(0, 0)),
	};
	vertex = new VKVertexBuffer(renderManager->GetVkInit(), &vertices);

	std::vector<uint16_t> indices{ 0, 1, 2, 3 };
	index = new VKIndexBuffer(renderManager->GetVkInit(), renderManager->GetCommandPool(), &indices);

	uniform = new VKUniformBuffer<glm::mat3>(renderManager->GetVkInit());
}

Texture::~Texture()
{
}

void Texture::Resize(glm::mat3 matrix_, const uint32_t frameIndex_)
{
	matrix = matrix_;
	uniform->UpdateUniform(&matrix, frameIndex_);
}
