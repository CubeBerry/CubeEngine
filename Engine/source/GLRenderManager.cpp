//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLRenderManager.cpp
#include "Engine.hpp"
//#include <glm/gtx/transform.hpp>

GLRenderManager::~GLRenderManager()
{
	//Delete ImGui
	delete imguiManager;

	//Destroy Buffers
	delete vertexBuffer;
	delete indexBuffer;
	delete vertexUniform2D;
	delete fragmentUniform2D;
	delete vertexUniform3D;
	delete fragmentUniform3D;
	delete fragmentMaterialUniformBuffer;
	delete vertexLightingUniformBuffer;

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

void GLRenderManager::Initialize(
	SDL_Window* window_, SDL_GLContext context_
)
{
	vertexArray.Initialize();
#ifdef _DEBUG
	normalVertexArray.Initialize();

	////Attributes
	//GLAttributeLayout position_layout;
	//position_layout.component_type = GLAttributeLayout::Float;
	//position_layout.component_dimension = GLAttributeLayout::_4;
	//position_layout.normalized = false;
	//position_layout.vertex_layout_location = 0;
	//position_layout.stride = sizeof(ThreeDimension::NormalVertex);
	//position_layout.offset = 0;
	//position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

	//GLAttributeLayout color_layout;
	//color_layout.component_type = GLAttributeLayout::Float;
	//color_layout.component_dimension = GLAttributeLayout::_4;
	//color_layout.normalized = false;
	//color_layout.vertex_layout_location = 1;
	//color_layout.stride = sizeof(ThreeDimension::NormalVertex);
	//color_layout.offset = 0;
	//color_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, color);

	//GLAttributeLayout index_layout;
	//index_layout.component_type = GLAttributeLayout::Int;
	//index_layout.component_dimension = GLAttributeLayout::_1;
	//index_layout.normalized = false;
	//index_layout.vertex_layout_location = 2;
	//index_layout.stride = sizeof(ThreeDimension::NormalVertex);
	//index_layout.offset = 0;
	//index_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, index);

	//normalVertexArray.AddVertexBuffer(std::move(*normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { position_layout, color_layout, index_layout });
#endif
	gl2DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/2D.vert" }, { GLShader::FRAGMENT, "../Engine/shader/2D.frag" } });
	gl3DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/3D.vert" }, { GLShader::FRAGMENT, "../Engine/shader/3D.frag" } });
#ifdef _DEBUG
	glNormal3DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/Normal3D.vert" }, { GLShader::FRAGMENT, "../Engine/shader/Normal3D.frag" } });
#endif

	vertexUniform2D = new GLUniformBuffer<TwoDimension::VertexUniform>();
	fragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();
	vertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms2D.size() * sizeof(TwoDimension::VertexUniform), vertexUniforms2D.data());
	fragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms2D.size() * sizeof(TwoDimension::FragmentUniform), fragUniforms2D.data());

	vertexUniform3D = new GLUniformBuffer<ThreeDimension::VertexUniform>();
	fragmentUniform3D = new GLUniformBuffer<ThreeDimension::FragmentUniform>();
	vertexUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms3D.size() * sizeof(ThreeDimension::VertexUniform), vertexUniforms3D.data());
	fragmentUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms3D.size() * sizeof(ThreeDimension::FragmentUniform), fragUniforms3D.data());

	//Lighting
	vertexLightingUniformBuffer = new GLUniformBuffer<ThreeDimension::VertexLightingUniform>();
	vertexLightingUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 3, "vLightingMatrix", sizeof(ThreeDimension::VertexLightingUniform), vertexLightingUniformBuffer);
	imguiManager = new GLImGuiManager(window_, context_);
}

void GLRenderManager::BeginRender(glm::vec3 bgColor)
{
	glCheck(glEnable(GL_DEPTH_TEST));
	glCheck(glDepthFunc(GL_LEQUAL));
	glCheck(glCullFace(GL_BACK));
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
		gl2DShader.Use(true);

		//For Texture Array
		if (!samplers.empty())
		{
			auto texLocation = glCheck(glGetUniformLocation(gl2DShader.GetProgramHandle(), "tex"));
			glCheck(glUniform1iv(texLocation, static_cast<GLsizei>(samplers.size()), samplers.data()));
		}

		if (vertexUniform2D != nullptr)
		{
			vertexUniform2D->UpdateUniform(vertexUniforms2D.size() * sizeof(TwoDimension::VertexUniform), vertexUniforms2D.data());
		}
		if (fragmentUniform2D != nullptr)
		{
			fragmentUniform2D->UpdateUniform(fragUniforms2D.size() * sizeof(TwoDimension::FragmentUniform), fragUniforms2D.data());
		}
		break;
	case RenderType::ThreeDimension:
		gl3DShader.Use(true);

		if (vertexUniform3D != nullptr)
		{
			vertexUniform3D->UpdateUniform(vertexUniforms3D.size() * sizeof(ThreeDimension::VertexUniform), vertexUniforms3D.data());
		}
		if (vertexLightingUniformBuffer != nullptr)
		{
			vertexLightingUniformBuffer->UpdateUniform(sizeof(ThreeDimension::VertexLightingUniform), &vertexLightingUniform);
		}
		if (fragmentUniform3D != nullptr)
		{
			fragmentUniform3D->UpdateUniform(fragUniforms3D.size() * sizeof(ThreeDimension::FragmentUniform), fragUniforms3D.data());
			fragmentMaterialUniformBuffer->UpdateUniform(fragMaterialUniforms3D.size() * sizeof(ThreeDimension::Material), fragMaterialUniforms3D.data());
		}
		break;
	}

	vertexArray.Use(true);
	GLDrawIndexed(vertexArray);
	vertexArray.Use(false);

	switch (rMode)
	{
	case RenderType::TwoDimension:
		gl2DShader.Use(false);
		break;
	case RenderType::ThreeDimension:
		gl3DShader.Use(false);
		break;
	}

#ifdef _DEBUG
	if (isDrawNormals)
	{
		glNormal3DShader.Use(true);
		normalVertexArray.Use(true);
		glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(normalVertices3D.size()));
		normalVertexArray.Use(false);
		glNormal3DShader.Use(false);
	}
#endif

	//Skybox
	if (skyboxEnabled)
	{
		skyboxShader.Use(true);
		if (vertexUniform3D != nullptr)
		{
			vertexUniform3D->UpdateUniform(vertexUniforms3D.size() * sizeof(ThreeDimension::VertexUniform), vertexUniforms3D.data());
		}
		skyboxVertexArray.Use(true);
		auto skyboxLocation = glCheck(glGetUniformLocation(skyboxShader.GetProgramHandle(), "skybox"));
		glCheck(glUniform1i(skyboxLocation, 0));
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetTextureHandle());
		glCheck(glDrawArrays(GL_TRIANGLES, 0, 36));
		skyboxVertexArray.Use(false);
		skyboxShader.Use(false);
	}

	imguiManager->Begin();
}

void GLRenderManager::EndRender()
{
	imguiManager->End();

	SDL_GL_SwapWindow(Engine::Instance().GetWindow().GetWindow());
}

void GLRenderManager::LoadTexture(const std::filesystem::path& path_, std::string name_)
{
	GLTexture* texture = new GLTexture;
	texture->LoadTexture(path_, name_, static_cast<int>(textures.size()));

	textures.push_back(texture);
	samplers.push_back(texture->GetTextrueId());

	//int texId = static_cast<int>(textures.size() - 1);
}

void GLRenderManager::LoadQuad(glm::vec4 color_, float isTex_, float isTexel_)
{
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(-1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(1.f, -1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec3(-1.f, -1.f, 1.f), quadCount));

	if (quadCount > 0)
		delete vertexBuffer;
	vertexBuffer = new GLVertexBuffer;
	vertexBuffer->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices2D.size()), vertices2D.data());

	uint64_t indexNumber{ vertices2D.size() / 4 - 1 };
	indices.push_back(static_cast<uint16_t>(4 * indexNumber));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 1));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber + 3));
	indices.push_back(static_cast<uint16_t>(4 * indexNumber));
	if (indexBuffer != nullptr)
		delete indexBuffer;
	indexBuffer = new GLIndexBuffer(&indices);

	quadCount++;

	//Attributes
	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_3;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(TwoDimension::Vertex);
	position_layout.offset = 0;
	position_layout.relative_offset = offsetof(TwoDimension::Vertex, position);

	GLAttributeLayout index_layout;
	index_layout.component_type = GLAttributeLayout::Int;
	index_layout.component_dimension = GLAttributeLayout::_1;
	index_layout.normalized = false;
	index_layout.vertex_layout_location = 1;
	index_layout.stride = sizeof(TwoDimension::Vertex);
	index_layout.offset = 0;
	index_layout.relative_offset = offsetof(TwoDimension::Vertex, index);

	vertexArray.AddVertexBuffer(std::move(*vertexBuffer), sizeof(TwoDimension::Vertex), {position_layout, index_layout});
	vertexArray.SetIndexBuffer(std::move(*indexBuffer));

	if (vertexUniform2D != nullptr)
		delete vertexUniform2D;
	vertexUniform2D = new GLUniformBuffer<TwoDimension::VertexUniform>();
	vertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms2D.size(), vertexUniforms2D.data());

	if (fragmentUniform2D != nullptr)
		delete fragmentUniform2D;
	fragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();
	fragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms2D.size(), fragUniforms2D.data());


	TwoDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexUniforms2D.push_back(mat);
	vertexUniforms2D.back().color = color_;
	vertexUniforms2D.back().isTex = isTex_;
	vertexUniforms2D.back().isTexel = isTexel_;

	TwoDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragUniforms2D.push_back(tIndex);
}

void GLRenderManager::DeleteWithIndex(int id)
{
	quadCount--;

	if (quadCount == 0)
	{
		switch (rMode)
		{
		case RenderType::TwoDimension:
			vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
			break;
		case RenderType::ThreeDimension:
			vertices3D.erase(end(vertices3D) - *verticesPerMesh.begin(), end(vertices3D));
#ifdef _DEBUG
			normalVertices3D.erase(end(normalVertices3D) - *normalVerticesPerMesh.begin(), end(normalVertices3D));
			delete normalVertexBuffer;
			normalVertexBuffer = nullptr;
#endif
			break;
		}
		delete vertexBuffer;
		vertexBuffer = nullptr;

		switch (rMode)
		{
		case RenderType::TwoDimension:
			indices.erase(end(indices) - 6, end(indices));
			break;
		case RenderType::ThreeDimension:
			indices.erase(end(indices) - *indicesPerMesh.begin(), end(indices));
			break;
		}
		delete indexBuffer;
		indexBuffer = nullptr;

		if (rMode == RenderType::ThreeDimension)
		{
			verticesPerMesh.erase(verticesPerMesh.begin());
#ifdef _DEBUG
			normalVerticesPerMesh.erase(normalVerticesPerMesh.begin());
#endif
			indicesPerMesh.erase(indicesPerMesh.begin());
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
			delete vertexUniform2D;
			vertexUniform2D = nullptr;
			break;
		case RenderType::ThreeDimension:
			vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
			delete vertexUniform3D;
			vertexUniform3D = nullptr;
			break;
		}

		switch (rMode)
		{
		case RenderType::TwoDimension:
			fragUniforms2D.erase(end(fragUniforms2D) - 1);
			delete fragmentUniform2D;
			fragmentUniform2D = nullptr;
			break;
		case RenderType::ThreeDimension:
			fragUniforms3D.erase(end(fragUniforms3D) - 1);
			delete fragmentUniform3D;
			fragmentUniform3D = nullptr;

			fragMaterialUniforms3D.erase(end(fragMaterialUniforms3D) - 1);
			delete fragmentMaterialUniformBuffer;
			fragmentMaterialUniformBuffer = nullptr;
			break;
		}

		//Destroy Texture
		for (auto t : textures)
			delete t;
		textures.erase(textures.begin(), textures.end());
		samplers.erase(samplers.begin(), samplers.end());

		return;
	}

	switch (rMode)
	{
	case RenderType::TwoDimension:
		vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
		indices.erase(end(indices) - 6, end(indices));
		//glCheck(glNamedBufferSubData(vertexBuffer->GetHandle(), 0, static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * vertices2D.size()), vertices2D.data()));
		//glCheck(glNamedBufferSubData(indexBuffer->GetHandle(), 0, sizeof(uint16_t) * indices.size(), indices.data()));
		break;
	case RenderType::ThreeDimension:
		unsigned int beginCount{ 0 };
		for (int v = 0; v < id; ++v)
		{
			beginCount += verticesPerMesh[v];
		}
		vertices3D.erase(begin(vertices3D) + beginCount, begin(vertices3D) + beginCount + verticesPerMesh[id]);
		for (auto it = vertices3D.begin() + beginCount; it != vertices3D.end(); ++it)
		{
			it->index--;
		}

		beginCount = 0;
		for (int v = 0; v < id; ++v)
		{
			beginCount += indicesPerMesh[v];
		}
		indices.erase(begin(indices) + beginCount, begin(indices) + beginCount + indicesPerMesh[id]);
		for (auto it = indices.begin() + beginCount; it != indices.end(); ++it)
		{
			(*it) = (*it) - static_cast<unsigned short>(verticesPerMesh[id]);
		}

		glCheck(glNamedBufferSubData(vertexBuffer->GetHandle(), 0, static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices3D.size()), vertices3D.data()));
		glCheck(glNamedBufferSubData(indexBuffer->GetHandle(), 0, sizeof(uint16_t) * indices.size(), indices.data()));
		vertexArray.SetIndexBuffer(std::move(*indexBuffer));

#ifdef _DEBUG
		beginCount = 0;
		for (int vn = 0; vn < id; ++vn)
		{
			beginCount += normalVerticesPerMesh[vn];
		}

		normalVertices3D.erase(begin(normalVertices3D) + beginCount, begin(normalVertices3D) + beginCount + normalVerticesPerMesh[id]);
		for (auto it = normalVertices3D.begin() + beginCount; it != normalVertices3D.end(); ++it)
		{
			it->index--;
		}

		glCheck(glNamedBufferSubData(normalVertexBuffer->GetHandle(), 0, static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex)* normalVertices3D.size()), normalVertices3D.data()));
#endif
		break;
	}

	if (rMode == RenderType::ThreeDimension)
	{
		verticesPerMesh.erase(verticesPerMesh.begin() + id);
		indicesPerMesh.erase(indicesPerMesh.begin() + id);
#ifdef _DEBUG
		normalVerticesPerMesh.erase(normalVerticesPerMesh.begin() + id);
#endif
	}

	switch (rMode)
	{
	case RenderType::TwoDimension:
		vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
		break;
	case RenderType::ThreeDimension:
		vertexUniforms3D.erase(end(vertexUniforms3D) - 1);
		break;
	}

	switch (rMode)
	{
	case RenderType::TwoDimension:
		fragUniforms2D.erase(end(fragUniforms2D) - 1);
		break;
	case RenderType::ThreeDimension:
		fragUniforms3D.erase(end(fragUniforms3D) - 1);
		fragMaterialUniforms3D.erase(end(fragMaterialUniforms3D) - 1);
		break;
	}
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

void GLRenderManager::LoadMesh(MeshType type, const std::filesystem::path& path, glm::vec4 color, int stacks, int slices, float shininess, glm::vec3 specularColor)
{
	CreateMesh(type, path, stacks, slices);

	if (quadCount > 0)
		delete vertexBuffer;
	vertexBuffer = new GLVertexBuffer;
	vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices3D.size()), vertices3D.data());
#ifdef _DEBUG
	normalVertexBuffer = new GLVertexBuffer;
	normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices3D.size()), normalVertices3D.data());
#endif

	if (indexBuffer != nullptr)
		delete indexBuffer;
	indexBuffer = new GLIndexBuffer(&indices);

	quadCount++;

	//Attributes
	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_3;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(ThreeDimension::Vertex);
	position_layout.offset = 0;
	position_layout.relative_offset = offsetof(ThreeDimension::Vertex, position);

	GLAttributeLayout normal_layout;
	normal_layout.component_type = GLAttributeLayout::Float;
	normal_layout.component_dimension = GLAttributeLayout::_3;
	normal_layout.normalized = false;
	normal_layout.vertex_layout_location = 1;
	normal_layout.stride = sizeof(ThreeDimension::Vertex);
	normal_layout.offset = 0;
	normal_layout.relative_offset = offsetof(ThreeDimension::Vertex, normal);

	GLAttributeLayout uv_layout;
	uv_layout.component_type = GLAttributeLayout::Float;
	uv_layout.component_dimension = GLAttributeLayout::_2;
	uv_layout.normalized = false;
	uv_layout.vertex_layout_location = 2;
	uv_layout.stride = sizeof(ThreeDimension::Vertex);
	uv_layout.offset = 0;
	uv_layout.relative_offset = offsetof(ThreeDimension::Vertex, uv);

	GLAttributeLayout index_layout;
	index_layout.component_type = GLAttributeLayout::Int;
	index_layout.component_dimension = GLAttributeLayout::_1;
	index_layout.normalized = false;
	index_layout.vertex_layout_location = 3;
	index_layout.stride = sizeof(ThreeDimension::Vertex);
	index_layout.offset = 0;
	index_layout.relative_offset = offsetof(ThreeDimension::Vertex, index);

	vertexArray.AddVertexBuffer(std::move(*vertexBuffer), sizeof(ThreeDimension::Vertex), { position_layout, normal_layout, uv_layout, index_layout });
	vertexArray.SetIndexBuffer(std::move(*indexBuffer));
#ifdef _DEBUG
	//Attributes
	GLAttributeLayout normal_position_layout;
	normal_position_layout.component_type = GLAttributeLayout::Float;
	normal_position_layout.component_dimension = GLAttributeLayout::_3;
	normal_position_layout.normalized = false;
	normal_position_layout.vertex_layout_location = 0;
	normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
	normal_position_layout.offset = 0;
	normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

	GLAttributeLayout normal_color_layout;
	normal_color_layout.component_type = GLAttributeLayout::Float;
	normal_color_layout.component_dimension = GLAttributeLayout::_4;
	normal_color_layout.normalized = false;
	normal_color_layout.vertex_layout_location = 1;
	normal_color_layout.stride = sizeof(ThreeDimension::NormalVertex);
	normal_color_layout.offset = 0;
	normal_color_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, color);

	GLAttributeLayout normal_index_layout;
	normal_index_layout.component_type = GLAttributeLayout::Int;
	normal_index_layout.component_dimension = GLAttributeLayout::_1;
	normal_index_layout.normalized = false;
	normal_index_layout.vertex_layout_location = 2;
	normal_index_layout.stride = sizeof(ThreeDimension::NormalVertex);
	normal_index_layout.offset = 0;
	normal_index_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, index);

	normalVertexArray.AddVertexBuffer(std::move(*normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout, normal_color_layout, normal_index_layout });
#endif

	if (vertexUniform3D != nullptr)
		delete vertexUniform3D;
	vertexUniform3D = new GLUniformBuffer<ThreeDimension::VertexUniform>();
	vertexUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms3D.size(), vertexUniforms3D.data());

	if (fragmentUniform3D != nullptr)
		delete fragmentUniform3D;
	fragmentUniform3D = new GLUniformBuffer<ThreeDimension::FragmentUniform>();
	fragmentUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms3D.size(), fragUniforms3D.data());

	if (fragmentMaterialUniformBuffer != nullptr)
		delete fragmentMaterialUniformBuffer;
	fragmentMaterialUniformBuffer = new GLUniformBuffer<ThreeDimension::Material>();
	fragmentMaterialUniformBuffer->InitUniform(gl3DShader.GetProgramHandle(), 2, "fUniformMatrix", fragUniforms3D.size(), fragUniforms3D.data());

	ThreeDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexUniforms3D.push_back(mat);
	vertexUniforms3D.back().color = color;

	ThreeDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragUniforms3D.push_back(tIndex);

	ThreeDimension::Material material;
	material.shininess = shininess;
	material.specularColor = specularColor;
	fragMaterialUniforms3D.push_back(material);
}

void GLRenderManager::LoadSkyBox(
	const std::filesystem::path& right,
	const std::filesystem::path& left,
	const std::filesystem::path& top,
	const std::filesystem::path& bottom,
	const std::filesystem::path& front,
	const std::filesystem::path& back
)
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

	skyboxShader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/Skybox.vert" }, { GLShader::FRAGMENT, "../Engine/shader/Skybox.frag" } });
	skybox = new GLTexture;
	skybox->LoadSkyBox(right, left, top, bottom, front, back);
	skyboxEnabled = true;
}

void GLRenderManager::DeleteSkyBox()
{
	delete skyboxVertexBuffer;
	delete skybox;
	skyboxEnabled = false;
}
