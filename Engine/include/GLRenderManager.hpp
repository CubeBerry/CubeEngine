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

	GLShader gl2DShader;
	GLShader gl3DShader;
	GLImGuiManager* imguiManager;
public:
	//--------------------Common--------------------//
	void DeleteWithIndex(int id) override;
//	GLVertexBuffer<Vertex>* AllocateVertexBuffer(std::vector<Vertex>& vertices) const
//	{
//		return new GLVertexBuffer<Vertex>(vkInit, &vertices);
//	}
//#ifdef _DEBUG
//	GLVertexBuffer<ThreeDimension::NormalVertex>* AllocateNormalVertexBuffer(std::vector<ThreeDimension::NormalVertex>& vertices) const
//	{
//		return new GLVertexBuffer<ThreeDimension::NormalVertex>(vkInit, &vertices);
//	}
//#endif
//	GLIndexBuffer* AllocateIndexBuffer(std::vector<uint32_t>& indices)
//	{
//		return new GLIndexBuffer(vkInit, &vkCommandPool, &indices);
//	}
//	[[nodiscard]] GLUniformBuffer<VertexUniform>* AllocateVertexUniformBuffer() const
//	{
//		return new GLUniformBuffer<VertexUniform>(vkInit, 1);
//	}
//	[[nodiscard]] GLUniformBuffer<FragmentUniform>* AllocateFragmentUniformBuffer() const
//	{
//		return new GLUniformBuffer<FragmentUniform>(vkInit, 1);
//	}
//	[[nodiscard]] GLUniformBuffer<ThreeDimension::Material>* AllocateMaterialUniformBuffer() const
//	{
//		return new GLUniformBuffer<ThreeDimension::Material>(vkInit, 1);
//	}

	//--------------------2D Render--------------------//
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;

	GLTexture* GetTexture(std::string name);
	std::vector<GLTexture*> GetTextures() { return textures; }

	//--------------------3D Render--------------------//

	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
private:
	//--------------------Common--------------------//
	std::vector<GLTexture*> textures;
	std::vector<int> samplers;

#ifdef _DEBUG
	GLVertexArray normalVertexArray;
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
