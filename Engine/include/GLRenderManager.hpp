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

class GLSkybox;

class GLRenderManager : public RenderManager
{
public:
	GLRenderManager() { gMode = GraphicsMode::GL; };
	~GLRenderManager();
	void Initialize(
		SDL_Window* window_, SDL_GLContext context_
	);

	bool BeginRender(glm::vec3 bgColor) override;
	void EndRender() override;
private:
	void GLDrawIndexed(const GLVertexArray& vertex_array) noexcept
	{
		glCheck(glDrawElements(GL_TRIANGLES, vertex_array.GetIndicesCount(), GL_UNSIGNED_INT, 0));
	}

	void GLDrawVertices(const GLVertexArray& vertex_array) noexcept
	{
		glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_array.GetVerticesCount()));
	}

	GLVertexArray vertexArray;
	GLShader gl2DShader;
	GLShader gl3DShader;
	GLImGuiManager* imguiManager;
public:
	//--------------------Common--------------------//
	void DeleteWithIndex(int id) override;

	//--------------------2D Render--------------------//
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;

	GLTexture* GetTexture(std::string name);
	std::vector<GLTexture*> GetTextures() { return textures; }

	//--------------------3D Render--------------------//

	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
private:
	//--------------------Common--------------------//
	GLVertexBuffer* vertexBuffer{ nullptr };
	GLIndexBuffer* indexBuffer{ nullptr };

	//--------------------2D Render--------------------//
	std::vector<GLTexture*> textures;
	std::vector<int> samplers;

	GLUniformBuffer<TwoDimension::VertexUniform>* vertexUniform2D{ nullptr };
	GLUniformBuffer<TwoDimension::FragmentUniform>* fragmentUniform2D{ nullptr };

	//--------------------3D Render--------------------//
	GLUniformBuffer<ThreeDimension::VertexUniform>* vertexUniform3D{ nullptr };
	GLUniformBuffer<ThreeDimension::FragmentUniform>* fragmentUniform3D{ nullptr };
	GLUniformBuffer<ThreeDimension::Material>* fragmentMaterialUniformBuffer{ nullptr };

#ifdef _DEBUG
	GLVertexArray normalVertexArray;
	GLVertexBuffer* normalVertexBuffer{ nullptr };
	GLShader glNormal3DShader;
#endif

	//Lighting
	GLUniformBuffer<ThreeDimension::DirectionalLightUniform>* directionalLightUniformBuffer{ nullptr };
	GLUniformBuffer<ThreeDimension::PointLightUniform>* pointLightUniformBuffer{ nullptr };

	//Skybox
	GLVertexArray skyboxVertexArray;
	GLVertexBuffer* skyboxVertexBuffer{ nullptr };
	GLShader skyboxShader;
	GLSkybox* skybox;
};
