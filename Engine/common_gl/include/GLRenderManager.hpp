//Author: JEYOON YU
//Project: CubeEngine
//File: GLRenderManager.hpp
#include "GLShader.hpp"
#include "GLVertexArray.hpp"
#include "GLTexture.hpp"
#include "GLUniformBuffer.hpp"
#include "glm/vec4.hpp"
#include "Vertex.hpp"

class GLRenderManager
{
public:
	GLRenderManager() = default;
	~GLRenderManager();
	void Initialize();

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
	
	std::vector<Vertex> texVertices;
	GLVertexBuffer* texVertex{ nullptr };
	std::vector<uint16_t> texIndices;
	GLIndexBuffer* texIndex{ nullptr };

	std::vector<VertexUniform> vertexVector;
	GLUniformBuffer* uVertex{ nullptr };
	std::vector<FragmentUniform> fragVector;
	GLUniformBuffer* uFragment{ nullptr };

	unsigned int quadCount{ 0 };
};

void GLDrawIndexed(const GLVertexArray& vertex_array) noexcept
{
	glDrawElements(GL_TRIANGLES, vertex_array.GetIndicesCount(), GL_UNSIGNED_SHORT, 0);
}

void GLDrawVertices(const GLVertexArray& vertex_array) noexcept
{
	glDrawArrays(GL_TRIANGLES, 0, vertex_array.GetVerticesCount());
}
