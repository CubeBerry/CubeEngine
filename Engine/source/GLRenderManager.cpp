//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLRenderManager.cpp
#include "Engine.hpp"

GLRenderManager::~GLRenderManager()
{
#ifdef _DEBUG
	//Delete ImGui
	delete imguiManager;
#endif

	//Destroy Buffers
	delete vertexBuffer;
	delete indexBuffer;
	delete vertexUniform2D;
	delete fragmentUniform2D;

	//Destroy Texture
	for (const auto t : textures)
		delete t;
}

void GLRenderManager::Initialize(
#ifdef _DEBUG
	SDL_Window* window_, SDL_GLContext context_
#endif
)
{
	vertexArray.Initialize();
	gl2DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/OpenGL2D.vert" }, { GLShader::FRAGMENT, "../Engine/shader/OpenGL2D.frag" } });

	vertexUniform2D = new GLUniformBuffer<TwoDimension::VertexUniform>();
	fragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();

	vertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms2D);
	fragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms2D);
#ifdef _DEBUG
	imguiManager = new GLImGuiManager(window_, context_);
#endif
}

void GLRenderManager::BeginRender(glm::vec4 bgColor)
{
	glCheck(glEnable(GL_DEPTH_TEST));
	glCheck(glDepthFunc(GL_LEQUAL));
	glCheck(glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a));
	glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	gl2DShader.Use(true);

	//For Texture Array
	auto texLocation = glCheck(glGetUniformLocation(gl2DShader.GetProgramHandle(), "tex"));
	glCheck(glUniform1iv(texLocation, static_cast<GLsizei>(samplers.size()), samplers.data()));

	if (vertexUniform2D != nullptr)
	{
		vertexUniform2D->UpdateUniform(vertexUniforms2D);
	}
	if (fragmentUniform2D != nullptr)
	{
		fragmentUniform2D->UpdateUniform(fragUniforms2D);
	}

	vertexArray.Use(true);

	GLDrawIndexed(vertexArray);

	vertexArray.Use(false);
	gl2DShader.Use(false);

#ifdef _DEBUG
	imguiManager->Begin();
#endif
}

void GLRenderManager::EndRender()
{
#ifdef _DEBUG
	imguiManager->End();
#endif

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
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), quadCount));
	vertices2D.push_back(TwoDimension::Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), quadCount));

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
	position_layout.component_dimension = GLAttributeLayout::_4;
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
	vertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms2D);

	if (fragmentUniform2D != nullptr)
		delete fragmentUniform2D;
	fragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();
	fragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms2D);


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

void GLRenderManager::DeleteWithIndex()
{
	quadCount--;

	if (quadCount == 0)
	{
		vertices2D.erase(end(vertices2D) - 4, end(vertices2D));
		delete vertexBuffer;
		vertexBuffer = nullptr;

		indices.erase(end(indices) - 6, end(indices));
		delete indexBuffer;
		indexBuffer = nullptr;

		vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
		delete vertexUniform2D;
		vertexUniform2D = nullptr;

		fragUniforms2D.erase(end(fragUniforms2D) - 1);
		delete fragmentUniform2D;
		fragmentUniform2D = nullptr;

		//Destroy Texture
		for (auto t : textures)
			delete t;
		textures.erase(textures.begin(), textures.end());
		samplers.erase(samplers.begin(), samplers.end());

		return;
	}

	vertices2D.erase(end(vertices2D) - 4, end(vertices2D));

	indices.erase(end(indices) - 6, end(indices));

	vertexUniforms2D.erase(end(vertexUniforms2D) - 1);
	//for (auto u : *uVertex->GetUniformBuffers())
	//{
	//	vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(VertexUniform), vertexVector.data());
	//}

	fragUniforms2D.erase(end(fragUniforms2D) - 1);
	//for (auto u : *uFragment->GetUniformBuffers())
	//{
	//	vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(FragmentUniform), fragVector.data());
	//}
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

void GLRenderManager::LoadMesh(MeshType type)
{
	type;
}
