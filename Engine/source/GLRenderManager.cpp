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
	delete VertexBuffer2D;
	delete IndexBuffer2D;
	delete VertexUniform2D;
	delete FragmentUniform2D;

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

	VertexUniform2D = new GLUniformBuffer<TwoDimension::VertexUniform>();
	FragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();

	VertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexVector);
	FragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragVector);
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

	if (VertexUniform2D != nullptr)
	{
		VertexUniform2D->UpdateUniform(vertexVector);
	}
	if (FragmentUniform2D != nullptr)
	{
		FragmentUniform2D->UpdateUniform(fragVector);
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
	texVertices.push_back(TwoDimension::Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), quadCount));
	texVertices.push_back(TwoDimension::Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), quadCount));
	texVertices.push_back(TwoDimension::Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), quadCount));
	texVertices.push_back(TwoDimension::Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), quadCount));

	if (quadCount > 0)
		delete VertexBuffer2D;
	VertexBuffer2D = new GLVertexBuffer;
	VertexBuffer2D->SetData(static_cast<GLsizei>(sizeof(TwoDimension::Vertex) * texVertices.size()), texVertices.data());

	uint64_t indexNumber{ texVertices.size() / 4 - 1 };
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 1));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 3));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber));
	if (IndexBuffer2D != nullptr)
		delete IndexBuffer2D;
	IndexBuffer2D = new GLIndexBuffer(&texIndices);

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

	vertexArray.AddVertexBuffer(std::move(*VertexBuffer2D), sizeof(TwoDimension::Vertex), {position_layout, index_layout});
	vertexArray.SetIndexBuffer(std::move(*IndexBuffer2D));

	if (VertexUniform2D != nullptr)
		delete VertexUniform2D;
	VertexUniform2D = new GLUniformBuffer<TwoDimension::VertexUniform>();
	VertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexVector);

	if (FragmentUniform2D != nullptr)
		delete FragmentUniform2D;
	FragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();
	FragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragVector);


	TwoDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexVector.push_back(mat);
	vertexVector.back().color = color_;
	vertexVector.back().isTex = isTex_;
	vertexVector.back().isTexel = isTexel_;

	TwoDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragVector.push_back(tIndex);
}

void GLRenderManager::DeleteWithIndex()
{
	quadCount--;

	if (quadCount == 0)
	{
		texVertices.erase(end(texVertices) - 4, end(texVertices));
		delete VertexBuffer2D;
		VertexBuffer2D = nullptr;

		texIndices.erase(end(texIndices) - 6, end(texIndices));
		delete IndexBuffer2D;
		IndexBuffer2D = nullptr;

		vertexVector.erase(end(vertexVector) - 1);
		delete VertexUniform2D;
		VertexUniform2D = nullptr;

		fragVector.erase(end(fragVector) - 1);
		delete FragmentUniform2D;
		FragmentUniform2D = nullptr;

		//Destroy Texture
		for (auto t : textures)
			delete t;
		textures.erase(textures.begin(), textures.end());
		samplers.erase(samplers.begin(), samplers.end());

		return;
	}

	texVertices.erase(end(texVertices) - 4, end(texVertices));

	texIndices.erase(end(texIndices) - 6, end(texIndices));

	vertexVector.erase(end(vertexVector) - 1);
	//for (auto u : *uVertex->GetUniformBuffers())
	//{
	//	vkCmdUpdateBuffer(commandBuffer, u, 0, quadCount * sizeof(VertexUniform), vertexVector.data());
	//}

	fragVector.erase(end(fragVector) - 1);
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
