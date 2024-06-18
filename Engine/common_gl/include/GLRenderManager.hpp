//Author: JEYOON YU
//Project: CubeEngine
//File: GLRenderManager.hpp
#pragma once
#include "GLShader.hpp"
#include "GLVertexArray.hpp"
#include "GLTexture.hpp"
#include "GLUniformBuffer.hpp"
#include "glm/vec4.hpp"
#include "GLMaterial.hpp"

class GLRenderManager
{
public:
	GLRenderManager() = default;
	~GLRenderManager();
	void Initialize();

	void GLDrawIndexed(const GLVertexArray& vertex_array) noexcept
	{
		glDrawElements(GL_TRIANGLES, vertex_array.GetIndicesCount(), GL_UNSIGNED_SHORT, 0);
	}

	void GLDrawVertices(const GLVertexArray& vertex_array) noexcept
	{
		glDrawArrays(GL_TRIANGLES, 0, vertex_array.GetVerticesCount());
	}

	void BeginRender();
	void EndRender();
private:
	GLShader shader;

	//--------------------Texture Render--------------------//
public:
	void LoadTexture(const std::filesystem::path& path_, std::string name_);
	void LoadQuad(glm::vec4 color_, float isTex_, float isTexel_);
	//void LoadLineQuad(glm::vec4 color_);
	//void LoadVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);
	//void LoadLineVertices(std::vector<Vertex> vertices_, std::vector<uint64_t> indices_);

	void DeleteWithIndex();
private:
	GLVertexArray vertexArray;
	std::vector<GLTexture*> textures;
	
	std::vector<GLVertex> texVertices;
	GLVertexBuffer* texVertex{ nullptr };
	std::vector<uint16_t> texIndices;
	GLIndexBuffer* texIndex{ nullptr };

	std::vector<GLVertexUniform> vertexVector;
	GLUniformBuffer<GLVertexUniform>* uVertex{ nullptr };
	std::vector<GLFragmentUniform> fragVector;
	GLUniformBuffer<GLFragmentUniform>* uFragment{ nullptr };

	unsigned int quadCount{ 0 };
};
