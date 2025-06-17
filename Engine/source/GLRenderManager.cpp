//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLRenderManager.cpp
#include "Engine.hpp"
#include "GLSkybox.hpp"
#include <span>

GLRenderManager::~GLRenderManager()
{
	//Delete ImGui
	delete imguiManager;

	//Destroy Buffers
	delete directionalLightUniformBuffer;
	delete pointLightUniformBuffer;

	//Destroy Skybox
	if (skyboxEnabled)
	{
		delete skybox;
		delete skyboxVertexBuffer;
	}
}

void GLRenderManager::Initialize(SDL_Window* window_, SDL_GLContext context_)
{
//#ifdef _DEBUG
//	normalVertexArray.Initialize();
//#endif

	gl2DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/2D.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/2D.frag" } });
	gl3DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/3D.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/3D.frag" } });
#ifdef _DEBUG
	glNormal3DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/Normal3D.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/Normal3D.frag" } });
#endif

	//Lighting
	directionalLightUniformBuffer = new GLUniformBuffer<ThreeDimension::DirectionalLightUniform>();
	directionalLightUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 5, "fDirectionalLightList", 0, nullptr);
	pointLightUniformBuffer = new GLUniformBuffer<ThreeDimension::PointLightUniform>();
	pointLightUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 6, "fPointLightList", 0, nullptr);

	imguiManager = new GLImGuiManager(window_, context_);
}

bool GLRenderManager::BeginRender(glm::vec3 bgColor)
{
	glCheck(glEnable(GL_DEPTH_TEST));
	glCheck(glDepthFunc(GL_LEQUAL));
	GLsizei w, h;
	SDL_GetWindowSizeInPixels(Engine::GetWindow().GetWindow(), &w, &h);
	glViewport(0, 0, w, h);
	switch (pMode)
	{
	case PolygonType::FILL:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case PolygonType::LINE:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}
	glCheck(glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.f));
	glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	switch (rMode)
	{
	case RenderType::TwoDimension:
		glCheck(glDisable(GL_CULL_FACE));

		gl2DShader.Use(true);

		//For Texture Array
		if (!samplers.empty())
		{
			auto texLocation = glCheck(glGetUniformLocation(gl2DShader.GetProgramHandle(), "tex"));
			glCheck(glUniform1iv(texLocation, static_cast<GLsizei>(samplers.size()), samplers.data()));
		}

		gl2DShader.Use(false);

		break;
	case RenderType::ThreeDimension:
		glCheck(glEnable(GL_CULL_FACE));
		glCheck(glCullFace(GL_BACK));

		gl3DShader.Use(true);

		//For Texture Array
		if (!samplers.empty())
		{
			auto texLoc = glCheck(glGetUniformLocation(gl3DShader.GetProgramHandle(), "tex"));
			glCheck(glUniform1iv(texLoc, static_cast<GLsizei>(samplers.size()), samplers.data()));
		}

		GLint irradianceLoc = glGetUniformLocation(gl3DShader.GetProgramHandle(), "irradianceMap");
		GLint prefilterLoc = glGetUniformLocation(gl3DShader.GetProgramHandle(), "prefilterMap");
		GLint brdfLoc = glGetUniformLocation(gl3DShader.GetProgramHandle(), "brdfLUT");
		glUniform1i(irradianceLoc, 29);
		glUniform1i(prefilterLoc, 30);
		glUniform1i(brdfLoc, 31);

		//glActiveTexture(GL_TEXTURE9);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetIrradiance());
		glCheck(glBindTextureUnit(29, skybox->GetIrradiance()));
		//glActiveTexture(GL_TEXTURE10);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetPrefilter());
		glCheck(glBindTextureUnit(30, skybox->GetPrefilter()));
		//glActiveTexture(GL_TEXTURE11);
		//glBindTexture(GL_TEXTURE_2D, skybox->GetBRDF());
		glCheck(glBindTextureUnit(31, skybox->GetBRDF()));

		if (pointLightUniformBuffer != nullptr)
		{
			glCheck(glUniform1i(glGetUniformLocation(gl3DShader.GetProgramHandle(), "activePointLights"), static_cast<GLint>(pointLightUniforms.size())));
			pointLightUniformBuffer->UpdateUniform(pointLightUniforms.size() * sizeof(ThreeDimension::PointLightUniform), pointLightUniforms.data());
		}
		if (directionalLightUniformBuffer != nullptr)
		{
			glCheck(glUniform1i(glGetUniformLocation(gl3DShader.GetProgramHandle(), "activeDirectionalLights"), static_cast<GLint>(directionalLightUniforms.size())));
			directionalLightUniformBuffer->UpdateUniform(directionalLightUniforms.size() * sizeof(ThreeDimension::DirectionalLightUniform), directionalLightUniforms.data());
		}

		gl3DShader.Use(false);

		break;
	}

	std::vector<Sprite*> sprites = Engine::Instance().GetSpriteManager().GetSprites();
	for (auto& sprite : sprites)
	{
		auto& buffer = sprite->GetBufferWrapper()->GetBuffer<BufferWrapper::GLBuffer>();
		switch (rMode)
		{
		case RenderType::TwoDimension:
			gl2DShader.Use(true);

			sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().vertexUniformBuffer->UpdateUniform(sizeof(TwoDimension::VertexUniform), &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().vertexUniform);

			sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().fragmentUniformBuffer->UpdateUniform(sizeof(TwoDimension::FragmentUniform), &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData2D>().fragmentUniform);

			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().vertexUniformBuffer->GetHandle()));
			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 1, sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer2D>().fragmentUniformBuffer->GetHandle()));

			buffer.vertexArray.Use(true);
			GLDrawIndexed(buffer.vertexArray);
			buffer.vertexArray.Use(false);

			gl2DShader.Use(false);
			break;
		case RenderType::ThreeDimension:
			gl3DShader.Use(true);

			//auto& vertexUniformBuffer = std::get<BufferWrapper::GLBuffer>(sprite->GetBuffer()->buffer);
			sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().vertexUniformBuffer->UpdateUniform(sizeof(ThreeDimension::VertexUniform), &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform);

			//auto& fragmentUniformBuffer = std::get<GLUniformBuffer<FragmentUniform>*>(sprite->GetFragmentUniformBuffer()->buffer);
			sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().fragmentUniformBuffer->UpdateUniform(sizeof(ThreeDimension::FragmentUniform), &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform);

			//auto& materialUniformBuffer = std::get<GLUniformBuffer<ThreeDimension::Material>*>(sprite->GetMaterialUniformBuffer()->buffer);
			sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().materialUniformBuffer->UpdateUniform(sizeof(ThreeDimension::Material), &sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().material);

			buffer.vertexArray.Use(true);
			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 2, sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().vertexUniformBuffer->GetHandle()));
			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 3, sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().fragmentUniformBuffer->GetHandle()));
			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 4, sprite->GetBufferWrapper()->GetUniformBuffer<BufferWrapper::GLUniformBuffer3D>().materialUniformBuffer->GetHandle()));

			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 5, directionalLightUniformBuffer->GetHandle()));
			glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 6, pointLightUniformBuffer->GetHandle()));
			GLDrawIndexed(buffer.vertexArray);
			buffer.vertexArray.Use(false);

			gl3DShader.Use(false);

#ifdef _DEBUG
			//if (isDrawNormals)
			//{
			//	glNormal3DShader.Use(true);
			//	normalVertexArray.Use(true);
			//	//glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(normalVertices3D.size()));
			//	normalVertexArray.Use(false);
			//	glNormal3DShader.Use(false);
			//}

			if (isDrawNormals)
			{
				glNormal3DShader.Use(true);

				buffer.normalVertexArray.Use(true);
				GLsizei size = static_cast<GLsizei>(sprite->GetBufferWrapper()->GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices.size());
				glDrawArrays(GL_LINES, 0, size);
				buffer.normalVertexArray.Use(false);

				glNormal3DShader.Use(false);
			}
#endif
			break;
		}
	}

	//switch (rMode)
	//{
	//case RenderType::TwoDimension:
	//	gl2DShader.Use(false);
	//	break;
	//case RenderType::ThreeDimension:
	//	gl3DShader.Use(false);
	//	break;
	//}

	//Skybox
	if (skyboxEnabled)
	{
		skyboxShader.Use(true);
		//if (vertexUniform3D != nullptr)
		//{
		//	vertexUniform3D->UpdateUniform(vertexUniforms3D.size() * sizeof(ThreeDimension::VertexUniform), vertexUniforms3D.data());
		//}
		GLint viewLoc = glGetUniformLocation(skyboxShader.GetProgramHandle(), "view");
		GLint projectionLoc = glGetUniformLocation(skyboxShader.GetProgramHandle(), "projection");

		std::span<const float, 16> spanView(&Engine::GetCameraManager().GetViewMatrix()[0][0], 16);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, spanView.data());
		std::span<const float, 16> spanProjection(&Engine::GetCameraManager().GetProjectionMatrix()[0][0], 16);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, spanProjection.data());

		GLint skyboxLoc = glCheck(glGetUniformLocation(skyboxShader.GetProgramHandle(), "skybox"));
		glCheck(glUniform1i(skyboxLoc, 0));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetCubeMap());

		skyboxVertexArray.Use(true);
		glCheck(glDrawArrays(GL_TRIANGLES, 0, 36));
		skyboxVertexArray.Use(false);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		skyboxShader.Use(false);
	}

	imguiManager->Begin();

	return true;
}

void GLRenderManager::EndRender()
{
	imguiManager->End();

	SDL_GL_SwapWindow(Engine::Instance().GetWindow().GetWindow());
}

void GLRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_, bool flip)
{
	std::unique_ptr<GLTexture> texture = std::make_unique<GLTexture>();
	int texID = static_cast<int>(textures.size());
	texture->LoadTexture(false, path_, name_, flip, texID);

	textures.emplace_back(std::move(texture));
	samplers.push_back(texID);

	//int texId = static_cast<int>(textures.size() - 1);
}

void GLRenderManager::ClearTextures()
{
	textures.clear();
	samplers.erase(samplers.begin(), samplers.end());
}

GLTexture* GLRenderManager::GetTexture(std::string name)
{
	for (auto& tex : textures)
	{
		if (tex->GetName() == name)
		{
			return tex.get();
		}
	}
	return nullptr;
}

void GLRenderManager::LoadSkybox(const std::filesystem::path& path)
{
	skyboxVertexArray.Initialize();

	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	skyboxVertexBuffer = new GLVertexBuffer;
	skyboxVertexBuffer->SetData(sizeof(float) * 108, skyboxVertices);

	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_3;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(float) * 3;
	position_layout.offset = 0;
	position_layout.relative_offset = 0;

	skyboxVertexArray.AddVertexBuffer(std::move(*skyboxVertexBuffer), sizeof(float) * 3, { position_layout });

	skyboxShader.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/Skybox.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/Skybox.frag" } });
	skybox = new GLSkybox(path);

	//Revert GL_TEXTURE0 which is binded(covered) by BRDFLUT's texture when loading skybox to first loaded texture
	if (!textures.empty())
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]->GetTextureHandle());
	}

	skyboxEnabled = true;
}

void GLRenderManager::DeleteSkybox()
{
	delete skyboxVertexBuffer;
	delete skybox;
	skyboxEnabled = false;
}
