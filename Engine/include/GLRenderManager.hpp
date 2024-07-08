//Author: JEYOON YU
//Project: CubeEngine
//File: GLRenderManager.hpp
#pragma once
#include "glCheck.hpp"
#include "RenderManager.hpp"
#include "GLShader.hpp"
#include "GLVertexArray.hpp"
#include "GLTexture.hpp"
#include "GLUniformBuffer.hpp"
#include "GLImGuiManager.hpp"
#include "Material.hpp"

class GLRenderManager : public RenderManager
{
public:
	GLRenderManager() { gMode = GraphicsMode::GL; };
	~GLRenderManager();
	void Initialize(
#ifdef _DEBUG
		SDL_Window* window_, SDL_GLContext context_
#endif
	);

	void BeginRender() override;
	void EndRender() override;
private:
	void GLDrawIndexed(const GLVertexArray& vertex_array) noexcept
	{
		glCheck(glDrawElements(GL_TRIANGLES, vertex_array.GetIndicesCount(), GL_UNSIGNED_SHORT, 0));
	}

	void GLDrawVertices(const GLVertexArray& vertex_array) noexcept
	{
		glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_array.GetVerticesCount()));
	}

	GLVertexArray vertexArray;
	GLShader shader;
#ifdef _DEBUG
	GLImGuiManager* imguiManager;
#endif

	//--------------------Texture Render--------------------//
public:
	void LoadTexture(const std::filesystem::path& path_, std::string name_) override;
	void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_) override;
	//void LoadLineQuad(glm::vec4 color_);
	//void LoadVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);
	//void LoadLineVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);

	void DeleteWithIndex() override;

	GLTexture* GetTexture(std::string name);
private:
	std::vector<GLTexture*> textures;
	
	GLVertexBuffer* texVertex{ nullptr };
	GLIndexBuffer* texIndex{ nullptr };

	GLUniformBuffer<VertexUniform>* uVertex{ nullptr };
	GLUniformBuffer<FragmentUniform>* uFragment{ nullptr };
};
