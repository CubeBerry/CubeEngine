//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLRenderManager.cpp
#include "GLRenderManager.hpp"
#include "Engine.hpp"
#include "GLSkybox.hpp"
#include "BufferWrapper.hpp"
#include <span>

GLRenderManager::~GLRenderManager()
{
	//Delete ImGui
	delete imguiManager;

	//Destroy Buffers
	delete directionalLightUniformBuffer;
	delete pointLightUniformBuffer;

	//Destroy Skybox
	if (m_skyboxEnabled)
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
	GLuint normalBlockIndex = glGetUniformBlockIndex(glNormal3DShader.GetProgramHandle(), "vUniformMatrix");
	if (normalBlockIndex != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(glNormal3DShader.GetProgramHandle(), normalBlockIndex, 2);
	}
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
	glCheck(glDisable(GL_SCISSOR_TEST));
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

	std::vector<DynamicSprite*> sprites = Engine::Instance().GetSpriteManager().GetDynamicSprites();
	size_t cameraCount = Engine::GetCameraManager().GetCameraCount();

	for (size_t c = 0; c < cameraCount; ++c)
	{
		Camera* cam = Engine::GetCameraManager().GetCamera(static_cast<int>(c));
		if (cam == nullptr || !cam->GetIsActive()) continue;

		ViewportRect vp = cam->GetViewport();
		GLsizei w, h;
		SDL_GetWindowSizeInPixels(Engine::GetWindow().GetWindow(), &w, &h);
		GLint vx = static_cast<GLint>(vp.x * w);
		GLint vy = static_cast<GLint>((1.0f - vp.y - vp.height) * h);
		GLsizei vw = static_cast<GLsizei>(vp.width * w);
		GLsizei vh = static_cast<GLsizei>(vp.height * h);
		glViewport(vx, vy, vw, vh);

		// Clear only the camera's viewport region
		glEnable(GL_SCISSOR_TEST);
		glScissor(vx, vy, vw, vh);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);

		for (const auto& sprite : sprites)
		{
			for (auto& subMesh : sprite->GetSubMeshes())
			{
				auto* buffer = subMesh->GetBuffer<BufferWrapper::GLBuffer>();
				switch (rMode)
				{
				case RenderType::TwoDimension:
				{
					gl2DShader.Use(true);

					auto* spriteData = subMesh->GetData<BufferWrapper::DynamicSprite2D>();

					spriteData->vertexUniform.view = cam->GetViewMatrix();
					if (sprite->GetSpriteDrawType() == SpriteDrawType::UI)
					{
						glm::vec2 cameraViewSize = cam->GetViewSize();
						spriteData->vertexUniform.projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
						spriteData->vertexUniform.view = glm::mat4(1.f);
					}
					else
					{
						spriteData->vertexUniform.projection = cam->GetProjectionMatrix();
					}

					spriteData->GetVertexUniformBuffer<GLUniformBuffer<TwoDimension::VertexUniform>>()->UpdateUniform(sizeof(TwoDimension::VertexUniform), &spriteData->vertexUniform);
					spriteData->GetFragmentUniformBuffer<GLUniformBuffer<TwoDimension::FragmentUniform>>()->UpdateUniform(sizeof(TwoDimension::FragmentUniform), &spriteData->fragmentUniform);

					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, spriteData->GetVertexUniformBuffer<GLUniformBuffer<TwoDimension::VertexUniform>>()->GetHandle()));
					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 1, spriteData->GetFragmentUniformBuffer<GLUniformBuffer<TwoDimension::FragmentUniform>>()->GetHandle()));

					buffer->vertexArray->Use(true);
					GLDrawIndexed(*buffer->vertexArray);
					buffer->vertexArray->Use(false);

					gl2DShader.Use(false);
					break;
				}
				case RenderType::ThreeDimension:
				{
					gl3DShader.Use(true);

					auto* spriteData = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();

					spriteData->vertexUniform.view = cam->GetViewMatrix();
					spriteData->vertexUniform.projection = cam->GetProjectionMatrix();
					glm::mat4 inverseView = glm::inverse(spriteData->vertexUniform.view);
					spriteData->vertexUniform.viewPosition = glm::vec4(inverseView[3].x, inverseView[3].y, inverseView[3].z, 1.0f);

					//// Initialize bone matrices to identity for non-skeletal meshes
					//// This ensures all meshes render correctly, whether skinned or not
					//if (spriteData->boneInfoMap.empty())
					//{
					//	// Non-skeletal mesh: set all bone matrices to identity
					//	for (int i = 0; i < ThreeDimension::MAX_BONES; i++)
					//	{
					//		spriteData->vertexUniform.finalBones[i] = glm::mat4(1.0f);
					//	}
					//}
					// Only initialize bone matrices for non-skeletal meshes
					if (spriteData->boneInfoMap.empty())
					{
						for (int i = 0; i < ThreeDimension::MAX_BONES; i++)
						{
							spriteData->vertexUniform.finalBones[i] = glm::mat4(1.0f);
						}
					}

					spriteData->GetVertexUniformBuffer<GLUniformBuffer<ThreeDimension::VertexUniform>>()->UpdateUniform(sizeof(ThreeDimension::VertexUniform), &spriteData->vertexUniform);
					spriteData->GetFragmentUniformBuffer<GLUniformBuffer<ThreeDimension::FragmentUniform>>()->UpdateUniform(sizeof(ThreeDimension::FragmentUniform), &spriteData->fragmentUniform);
					spriteData->GetMaterialUniformBuffer<GLUniformBuffer<ThreeDimension::Material>>()->UpdateUniform(sizeof(ThreeDimension::Material), &spriteData->material);

					buffer->vertexArray->Use(true);

					for (int loc = 0; loc <= 8; ++loc)
					{
						GLint enabled, size, type, stride, bufferBinding;
						glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
						glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
						glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
						glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
						glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &bufferBinding);
					}

					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 2, spriteData->GetVertexUniformBuffer<GLUniformBuffer<ThreeDimension::VertexUniform>>()->GetHandle()));
					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 3, spriteData->GetFragmentUniformBuffer<GLUniformBuffer<ThreeDimension::FragmentUniform>>()->GetHandle()));
					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 4, spriteData->GetMaterialUniformBuffer<GLUniformBuffer<ThreeDimension::Material>>()->GetHandle()));

					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 5, directionalLightUniformBuffer->GetHandle()));
					glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 6, pointLightUniformBuffer->GetHandle()));
					GLDrawIndexed(*buffer->vertexArray);
					buffer->vertexArray->Use(false);

					gl3DShader.Use(false);

#ifdef _DEBUG
					if (m_normalVectorVisualization)
					{
						glNormal3DShader.Use(true);

						// Bind VertexUniform (binding 2) so Normal3D.vert can access finalBones[]
						glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 2, spriteData->GetVertexUniformBuffer<GLUniformBuffer<ThreeDimension::VertexUniform>>()->GetHandle()));

						buffer->normalVertexArray->Use(true);
						GLsizei size = static_cast<GLsizei>(spriteData->normalVertices.size());
						glDrawArrays(GL_LINES, 0, size);
						buffer->normalVertexArray->Use(false);

						glNormal3DShader.Use(false);
					}
#endif
					break;
				}
				}
			}
		}

		//Skybox
		if (m_skyboxEnabled)
		{
			skyboxShader.Use(true);
			GLint viewLoc = glGetUniformLocation(skyboxShader.GetProgramHandle(), "view");
			GLint projectionLoc = glGetUniformLocation(skyboxShader.GetProgramHandle(), "projection");

			std::span<const float, 16> spanView(&cam->GetViewMatrix()[0][0], 16);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, spanView.data());

			glm::mat4 projection = cam->GetProjectionMatrix();
			if (cam->GetCameraType() == CameraType::Orthographic)
			{
				GLsizei w, h;
				SDL_GetWindowSizeInPixels(Engine::GetWindow().GetWindow(), &w, &h);
				float aspect = (float)w / (float)h;
				projection = glm::perspective(glm::radians(cam->GetBaseFov()), aspect, 0.1f, 10.f);
			}

			std::span<const float, 16> spanProjection(&projection[0][0], 16);
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
	const auto& texture = textures.emplace_back(std::make_unique<GLTexture>());
	const int texID = static_cast<int>(textures.size() - 1);
	texture->LoadTexture(false, path_, name_, flip, texID);

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

	m_skyboxEnabled = true;
}

void GLRenderManager::DeleteSkybox()
{
	delete skyboxVertexBuffer;
	delete skybox;
	m_skyboxEnabled = false;
}
