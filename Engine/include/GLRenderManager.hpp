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

	void InitializeDynamicBuffers(BufferWrapper& bufferWrapper, std::vector<uint32_t>& indices) override
	{
		// Initialize Buffers
		auto* buffer = bufferWrapper.GetBuffer<BufferWrapper::GLBuffer>();
		buffer->vertexArray = std::make_unique<GLVertexArray>();
		buffer->vertexArray->Initialize();
		buffer->vertexBuffer = std::make_unique<GLVertexBuffer>();
#ifdef _DEBUG
		buffer->normalVertexArray = std::make_unique<GLVertexArray>();
		buffer->normalVertexArray->Initialize();
		buffer->normalVertexBuffer = std::make_unique<GLVertexBuffer>();
#endif
		buffer->indexBuffer = std::make_unique<GLIndexBuffer>(&indices);
		if (rMode == RenderType::TwoDimension)
		{
			auto* sprite = bufferWrapper.GetData<BufferWrapper::DynamicSprite2D>();
			sprite->SetVertexUniformBuffer(std::make_unique<GLUniformBuffer<TwoDimension::VertexUniform>>());
			sprite->SetFragmentUniformBuffer(std::make_unique<GLUniformBuffer<TwoDimension::FragmentUniform>>());
			sprite->GetVertexUniformBuffer<GLUniformBuffer<TwoDimension::VertexUniform>>()->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", sizeof(TwoDimension::VertexUniform), nullptr);
			sprite->GetFragmentUniformBuffer<GLUniformBuffer<TwoDimension::FragmentUniform>>()->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", sizeof(TwoDimension::FragmentUniform), nullptr);
		}
		else if (rMode == RenderType::ThreeDimension)
		{
			auto* sprite = bufferWrapper.GetData<BufferWrapper::DynamicSprite3DMesh>();
			sprite->SetVertexUniformBuffer(std::make_unique<GLUniformBuffer<ThreeDimension::VertexUniform>>());
			sprite->SetFragmentUniformBuffer(std::make_unique<GLUniformBuffer<ThreeDimension::FragmentUniform>>());
			sprite->SetMaterialUniformBuffer(std::make_unique<GLUniformBuffer<ThreeDimension::Material>>());
			sprite->GetVertexUniformBuffer<GLUniformBuffer<ThreeDimension::VertexUniform>>()->InitUniform(gl3DShader.GetProgramHandle(), 2, "vUniformMatrix", sizeof(ThreeDimension::VertexUniform), nullptr);
			sprite->GetFragmentUniformBuffer<GLUniformBuffer<ThreeDimension::FragmentUniform>>()->InitUniform(gl3DShader.GetProgramHandle(), 3, "fUniformMatrix", sizeof(ThreeDimension::FragmentUniform), nullptr);
			sprite->GetMaterialUniformBuffer<GLUniformBuffer<ThreeDimension::Material>>()->InitUniform(gl3DShader.GetProgramHandle(), 4, "fUniformMaterial", sizeof(ThreeDimension::Material), nullptr);
		}
	}

	// FidelityFX CAS
	// FidelityFX does not support OpenGL
	void UpdateScalePreset(const FidelityFX::UpscaleEffect& effect, const FfxFsr1QualityMode& mode, const FidelityFX::CASScalePreset& preset) override {}

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
