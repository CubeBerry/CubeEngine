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
	~GLRenderManager() override;
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
	void ClearTextures() override;


	void InitializeBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer = new GLVertexBuffer();
#ifdef _DEBUG
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer = new GLVertexBuffer();
#endif
		bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>().indexBuffer = new GLIndexBuffer(&indices);
		if (rMode == RenderType::TwoDimension)
		{
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().vertexUniformBuffer = new GLUniformBuffer<TwoDimension::VertexUniform>();
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().fragmentUniformBuffer = new GLUniformBuffer<TwoDimension::FragmentUniform>();
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().vertexUniformBuffer->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", sizeof(TwoDimension::VertexUniform), nullptr);
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().fragmentUniformBuffer->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", sizeof(TwoDimension::FragmentUniform), nullptr);
		}
		else if (rMode == RenderType::ThreeDimension)
		{
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().vertexUniformBuffer = new GLUniformBuffer<ThreeDimension::VertexUniform>();
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().fragmentUniformBuffer = new GLUniformBuffer<ThreeDimension::FragmentUniform>();
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().materialUniformBuffer = new GLUniformBuffer<ThreeDimension::Material>();
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().vertexUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 2, "vUniformMatrix", sizeof(ThreeDimension::VertexUniform), nullptr);
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().fragmentUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 3, "fUniformMatrix", sizeof(ThreeDimension::FragmentUniform), nullptr);
			bufferWrapper.GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().materialUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 4, "fUniformMaterial", sizeof(ThreeDimension::Material), nullptr);
		}
	}

	//--------------------2D Render--------------------//
	void LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip) override;

	GLTexture* GetTexture(std::string name);
	const std::vector<std::unique_ptr<GLTexture>>& GetTextures() { return textures; }

	//--------------------3D Render--------------------//

	void LoadSkybox(const std::filesystem::path& path) override;
	void DeleteSkybox() override;
private:
	//--------------------Common--------------------//
	std::vector<std::unique_ptr<GLTexture>> textures;
	std::vector<int> samplers;

#ifdef _DEBUG
	//GLVertexArray normalVertexArray;
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
