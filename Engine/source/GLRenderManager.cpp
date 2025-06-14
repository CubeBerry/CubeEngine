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

	//Destroy Texture
	for (const auto t : textures)
		delete t;

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
	GLTexture* texture = new GLTexture;
	texture->LoadTexture(false, path_, name_, flip, static_cast<int>(textures.size()));

	textures.push_back(texture);
	samplers.push_back(texture->GetTextrueId());

	//int texId = static_cast<int>(textures.size() - 1);
}

//void GLRenderManager::LoadQuad(glm::vec4 color_, float isTex_, float isTexel_)
//{
//	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(-1.f, 1.f, 1.f), quadCount));
//	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(1.f, 1.f, 1.f), quadCount));
//	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(1.f, -1.f, 1.f), quadCount));
//	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(-1.f, -1.f, 1.f), quadCount));
//
//	if (quadCount > 0)
//		delete vertexBuffer;
//	vertexBuffer = new GLVertexBuffer;
//	vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices2D.size()), vertices2D.data());
//
//	uint64_t indexNumber{ vertices2D.size() / 4 - 1 };
//	indices.push_back(static_cast<uint32_t>(4 * indexNumber));
//	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 1));
//	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 2));
//	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 2));
//	indices.push_back(static_cast<uint32_t>(4 * indexNumber + 3));
//	indices.push_back(static_cast<uint32_t>(4 * indexNumber));
//	if (indexBuffer != nullptr)
//		delete indexBuffer;
//	indexBuffer = new GLIndexBuffer(&indices);
//
//	quadCount++;
//
//	//Attributes
//	GLAttributeLayout position_layout;
//	position_layout.component_type = GLAttributeLayout::Float;
//	position_layout.component_dimension = GLAttributeLayout::_3;
//	position_layout.normalized = false;
//	position_layout.vertex_layout_location = 0;
//	position_layout.stride = sizeof(TwoDimension::Vertex);
//	position_layout.offset = 0;
//	position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);
//
//	GLAttributeLayout index_layout;
//	index_layout.component_type = GLAttributeLayout::Int;
//	index_layout.component_dimension = GLAttributeLayout::_1;
//	index_layout.normalized = false;
//	index_layout.vertex_layout_location = 1;
//	index_layout.stride = sizeof(TwoDimension::Vertex);
//	index_layout.offset = 0;
//	index_layout.relative_offset = offsetof(TwoDimension::Vertex, index);
//
//	vertexArray.AddVertexBuffer(std::move(*vertexBuffer), sizeof(TwoDimension::Vertex), {position_layout, index_layout});
//	vertexArray.SetIndexBuffer(std::move(*indexBuffer));
//
//	TwoDimension::VertexUniform mat;
//	mat.model = glm::mat4(1.f);
//	mat.view = glm::mat4(1.f);
//	mat.projection = glm::mat4(1.f);
//	vertexUniforms2D.push_back(mat);
//	vertexUniforms2D.back().color = color_;
//	vertexUniforms2D.back().isTex = isTex_;
//	vertexUniforms2D.back().isTexel = isTexel_;
//
//	TwoDimension::FragmentUniform tIndex;
//	tIndex.texIndex = 0;
//	fragUniforms2D.push_back(tIndex);
//}

void GLRenderManager::DeleteWithIndex(int /*id*/)
{
//	if (quadCount == 0)
//	{
//		switch (rMode)
//		{
//		case RenderType::TwoDimension:
//			vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
//			break;
//		case RenderType::ThreeDimension:
//			vertices3D.erase(end(vertices3D) - *verticesPerMesh.begin(), end(vertices3D));
//#ifdef _DEBUG
//			normalVertices3D.erase(end(normalVertices3D) - *normalVerticesPerMesh.begin(), end(normalVertices3D));
//			delete normalVertexBuffer;
//			normalVertexBuffer = nullptr;
//#endif
//			break;
//		}
//		delete vertexBuffer;
//		vertexBuffer = nullptr;
//
//		switch (rMode)
//		{
//		case RenderType::TwoDimension:
//			indices.erase(end(indices) - 6, end(indices));
//			break;
//		case RenderType::ThreeDimension:
//			indices.erase(end(indices) - *indicesPerMesh.begin(), end(indices));
//			break;
//		}
//		delete indexBuffer;
//		indexBuffer = nullptr;
//
//		if (rMode == RenderType::ThreeDimension)
//		{
//			verticesPerMesh.erase(verticesPerMesh.begin());
//#ifdef _DEBUG
//			normalVerticesPerMesh.erase(normalVerticesPerMesh.begin());
//#endif
//			indicesPerMesh.erase(indicesPerMesh.begin());
//		}
//
//		switch (rMode)
//		{
//		case RenderType::TwoDimension:
//			vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
//			//delete vertexUniform2D;
//			//vertexUniform2D = nullptr;
//			break;
//		case RenderType::ThreeDimension:
//			vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
//			//delete vertexUniform3D;
//			//vertexUniform3D = nullptr;
//			break;
//		}
//
//		switch (rMode)
//		{
//		case RenderType::TwoDimension:
//			fragUniforms2D.erase(end(fragUniforms2D) - 1);
//			//delete fragmentUniform2D;
//			//fragmentUniform2D = nullptr;
//			break;
//		case RenderType::ThreeDimension:
//			fragUniforms3D.erase(end(fragUniforms3D) - 1);
//			//delete fragmentUniform3D;
//			//fragmentUniform3D = nullptr;
//
//			fragMaterialUniforms3D.erase(end(fragMaterialUniforms3D) - 1);
//			//delete fragmentMaterialUniformBuffer;
//			//fragmentMaterialUniformBuffer = nullptr;
//			break;
//		}
//
		//Destroy Texture
		for (auto t : textures)
			delete t;
		textures.erase(textures.begin(), textures.end());
		samplers.erase(samplers.begin(), samplers.end());
//
//		return;
//	}
//
//	switch (rMode)
//	{
//	case RenderType::TwoDimension:
//		vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
//		indices.erase(end(indices) - 6, end(indices));
//		//glCheck(glNamedBufferSubData(vertexBuffer->GetHandle(), 0, static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices2D.size()), vertices2D.data()));
//		//glCheck(glNamedBufferSubData(indexBuffer->GetHandle(), 0, sizeof(uint32_t) * indices.size(), indices.data()));
//		break;
//	case RenderType::ThreeDimension:
//		unsigned int beginCount{ 0 };
//		for (int v = 0; v < id; ++v)
//		{
//			beginCount += verticesPerMesh[v];
//		}
//		vertices3D.erase(begin(vertices3D) + beginCount, begin(vertices3D) + beginCount + verticesPerMesh[id]);
//		for (auto it = vertices3D.begin() + beginCount; it != vertices3D.end(); ++it)
//		{
//			it->index--;
//		}
//
//		beginCount = 0;
//		for (int v = 0; v < id; ++v)
//		{
//			beginCount += indicesPerMesh[v];
//		}
//		indices.erase(begin(indices) + beginCount, begin(indices) + beginCount + indicesPerMesh[id]);
//		for (auto it = indices.begin() + beginCount; it != indices.end(); ++it)
//		{
//			(*it) = (*it) - static_cast<unsigned short>(verticesPerMesh[id]);
//		}
//
//		glCheck(glNamedBufferSubData(vertexBuffer->GetHandle(), 0, static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices3D.size()), vertices3D.data()));
//		glCheck(glNamedBufferSubData(indexBuffer->GetHandle(), 0, sizeof(uint32_t) * indices.size(), indices.data()));
//		indexBuffer->SetCount(static_cast<int>(indices.size()));
//		vertexArray.SetIndexBuffer(std::move(*indexBuffer));
//
//#ifdef _DEBUG
//		beginCount = 0;
//		for (int vn = 0; vn < id; ++vn)
//		{
//			beginCount += normalVerticesPerMesh[vn];
//		}
//
//		normalVertices3D.erase(begin(normalVertices3D) + beginCount, begin(normalVertices3D) + beginCount + normalVerticesPerMesh[id]);
//		for (auto it = normalVertices3D.begin() + beginCount; it != normalVertices3D.end(); ++it)
//		{
//			it->index--;
//		}
//
//		glCheck(glNamedBufferSubData(normalVertexBuffer->GetHandle(), 0, static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex)* normalVertices3D.size()), normalVertices3D.data()));
//#endif
//		break;
//	}
//
//	if (rMode == RenderType::ThreeDimension)
//	{
//		verticesPerMesh.erase(verticesPerMesh.begin() + id);
//		indicesPerMesh.erase(indicesPerMesh.begin() + id);
//#ifdef _DEBUG
//		normalVerticesPerMesh.erase(normalVerticesPerMesh.begin() + id);
//#endif
//	}
//
//	switch (rMode)
//	{
//	case RenderType::TwoDimension:
//		vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
//		break;
//	case RenderType::ThreeDimension:
//		vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
//		break;
//	}
//
//	switch (rMode)
//	{
//	case RenderType::TwoDimension:
//		fragUniforms2D.erase(end(fragUniforms2D) - 1);
//		break;
//	case RenderType::ThreeDimension:
//		fragUniforms3D.erase(end(fragUniforms3D) - 1);
//		fragMaterialUniforms3D.erase(end(fragMaterialUniforms3D) - 1);
//		break;
//	}
}

GLTexture* GLRenderManager::GetTexture(std::string name)
{
	for (auto& tex : textures)
	{
		if (tex->GetName() == name)
		{
			return tex;
		}
	}
	return nullptr;
}

//void GLRenderManager::LoadMesh(MeshType type, const std::filesystem::path& path, glm::vec4 color, int stacks, int slices, float metallic, float roughness)
//{
//	CreateMesh(type, path, stacks, slices);
//
//	if (quadCount > 0)
//		delete vertexBuffer;
//	vertexBuffer = new GLVertexBuffer;
//	vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices3D.size()), vertices3D.data());
//#ifdef _DEBUG
//	normalVertexBuffer = new GLVertexBuffer;
//	normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices3D.size()), normalVertices3D.data());
//#endif
//
//	if (indexBuffer != nullptr)
//		delete indexBuffer;
//	indexBuffer = new GLIndexBuffer(&indices);
//
//	quadCount++;
//
//	//Attributes
//	GLAttributeLayout position_layout;
//	position_layout.component_type = GLAttributeLayout::Float;
//	position_layout.component_dimension = GLAttributeLayout::_3;
//	position_layout.normalized = false;
//	position_layout.vertex_layout_location = 0;
//	position_layout.stride = sizeof(ThreeDimension::Vertex);
//	position_layout.offset = 0;
//	position_layout.relative_offset = offsetof(ThreeDimension::Vertex, position);
//
//	GLAttributeLayout normal_layout;
//	normal_layout.component_type = GLAttributeLayout::Float;
//	normal_layout.component_dimension = GLAttributeLayout::_3;
//	normal_layout.normalized = false;
//	normal_layout.vertex_layout_location = 1;
//	normal_layout.stride = sizeof(ThreeDimension::Vertex);
//	normal_layout.offset = 0;
//	normal_layout.relative_offset = offsetof(ThreeDimension::Vertex, normal);
//
//	GLAttributeLayout uv_layout;
//	uv_layout.component_type = GLAttributeLayout::Float;
//	uv_layout.component_dimension = GLAttributeLayout::_2;
//	uv_layout.normalized = false;
//	uv_layout.vertex_layout_location = 2;
//	uv_layout.stride = sizeof(ThreeDimension::Vertex);
//	uv_layout.offset = 0;
//	uv_layout.relative_offset = offsetof(ThreeDimension::Vertex, uv);
//
//	GLAttributeLayout index_layout;
//	index_layout.component_type = GLAttributeLayout::Int;
//	index_layout.component_dimension = GLAttributeLayout::_1;
//	index_layout.normalized = false;
//	index_layout.vertex_layout_location = 3;
//	index_layout.stride = sizeof(ThreeDimension::Vertex);
//	index_layout.offset = 0;
//	index_layout.relative_offset = offsetof(ThreeDimension::Vertex, index);
//
//	GLAttributeLayout tex_sub_index_layout;
//	tex_sub_index_layout.component_type = GLAttributeLayout::Int;
//	tex_sub_index_layout.component_dimension = GLAttributeLayout::_1;
//	tex_sub_index_layout.normalized = false;
//	tex_sub_index_layout.vertex_layout_location = 4;
//	tex_sub_index_layout.stride = sizeof(ThreeDimension::Vertex);
//	tex_sub_index_layout.offset = 0;
//	tex_sub_index_layout.relative_offset = offsetof(ThreeDimension::Vertex, texSubIndex);
//
//	vertexArray.AddVertexBuffer(std::move(*vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, index_layout, tex_sub_index_layout });
//	vertexArray.SetIndexBuffer(std::move(*indexBuffer));
//#ifdef _DEBUG
//	//Attributes
//	GLAttributeLayout normal_position_layout;
//	normal_position_layout.component_type = GLAttributeLayout::Float;
//	normal_position_layout.component_dimension = GLAttributeLayout::_3;
//	normal_position_layout.normalized = false;
//	normal_position_layout.vertex_layout_location = 0;
//	normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
//	normal_position_layout.offset = 0;
//	normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);
//
//	GLAttributeLayout normal_color_layout;
//	normal_color_layout.component_type = GLAttributeLayout::Float;
//	normal_color_layout.component_dimension = GLAttributeLayout::_4;
//	normal_color_layout.normalized = false;
//	normal_color_layout.vertex_layout_location = 1;
//	normal_color_layout.stride = sizeof(ThreeDimension::NormalVertex);
//	normal_color_layout.offset = 0;
//	normal_color_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, color);
//
//	GLAttributeLayout normal_index_layout;
//	normal_index_layout.component_type = GLAttributeLayout::Int;
//	normal_index_layout.component_dimension = GLAttributeLayout::_1;
//	normal_index_layout.normalized = false;
//	normal_index_layout.vertex_layout_location = 2;
//	normal_index_layout.stride = sizeof(ThreeDimension::NormalVertex);
//	normal_index_layout.offset = 0;
//	normal_index_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, index);
//
//	normalVertexArray.AddVertexBuffer(std::move(*normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout, normal_index_layout });
//#endif
//
//	ThreeDimension::VertexUniform mat;
//	mat.model = glm::mat4(1.f);
//	mat.view = glm::mat4(1.f);
//	mat.projection = glm::mat4(1.f);
//	vertexUniforms3D.push_back(mat);
//	vertexUniforms3D.back().color = color;
//
//	ThreeDimension::FragmentUniform tIndex;
//	tIndex.texIndex = 0;
//	fragUniforms3D.push_back(tIndex);
//
//	ThreeDimension::Material material;
//	material.metallic = metallic;
//	material.roughness = roughness;
//	fragMaterialUniforms3D.push_back(material);
//}

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
