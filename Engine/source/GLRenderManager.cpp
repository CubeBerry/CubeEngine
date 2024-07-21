//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLRenderManager.cpp
#include "GLRenderManager.hpp"

GLRenderManager::~GLRenderManager()
{
}

void GLRenderManager::Initialize(
#ifdef _DEBUG
	SDL_Window* window_, SDL_GLContext context_
#endif
)
{
	vertexArray.Initialize();
	shader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/texVertex_OpenGL.vert" }, { GLShader::FRAGMENT, "../Engine/shader/texFragment_OpenGL.frag" } });

	texVertex = new GLVertexBuffer;

	//uVertex = new GLUniformBuffer<VertexUniform>();
	//uFragment = new GLUniformBuffer<FragmentUniform>();

	//uVertex->InitUniform(shader.GetProgramHandle(), 0, "vUniformMatrix", vertexVector);
	//uFragment->InitUniform(shader.GetProgramHandle(), 1, "fUniformMatrix", fragVector);
#ifdef _DEBUG
	imguiManager = new GLImGuiManager(window_, context_);
#endif
}

void GLRenderManager::BeginRender()
{
	glCheck(glClearColor(1, 0, 1, 1));
	glCheck(glClear(GL_COLOR_BUFFER_BIT));

	shader.Use(true);

	//For Texture Array
	auto texLocation = glCheck(glGetUniformLocation(shader.GetProgramHandle(), "tex"));
	glCheck(glUniform1iv(texLocation, static_cast<GLsizei>(samplers.size()), samplers.data()));

	//if (uVertex != nullptr)
	//{
	//	uVertex->UpdateUniform(vertexVector);
	//}
	//if (uFragment != nullptr)
	//{
	//	uFragment->UpdateUniform(fragVector);
	//}

	vertexArray.Use(true);

	GLDrawIndexed(vertexArray);

	vertexArray.Use(false);
	shader.Use(false);

#ifdef _DEBUG
	imguiManager->Begin();
#endif
}

void GLRenderManager::EndRender()
{
#ifdef _DEBUG
	imguiManager->End();
#endif
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
	if (texVertices.empty())
	{
		texVertices.push_back(Vertex(glm::vec4(-0.7f, 0.4f, 1.f, 1.f), quadCount));
		texVertices.push_back(Vertex(glm::vec4(-0.3f, 0.4f, 1.f, 1.f), quadCount));
		texVertices.push_back(Vertex(glm::vec4(-0.3f, -0.4f, 1.f, 1.f), quadCount));
		texVertices.push_back(Vertex(glm::vec4(-0.7f, -0.4f, 1.f, 1.f), quadCount));

	}
	else
	{
		texVertices.push_back(Vertex(glm::vec4(0.3f, 0.4f, 1.f, 1.f), quadCount));
		texVertices.push_back(Vertex(glm::vec4(0.7f, 0.4f, 1.f, 1.f), quadCount));
		texVertices.push_back(Vertex(glm::vec4(0.7f, -0.4f, 1.f, 1.f), quadCount));
		texVertices.push_back(Vertex(glm::vec4(0.3f, -0.4f, 1.f, 1.f), quadCount));
	}

	//texVertices.push_back(Vertex(glm::vec4(-1.f, 1.f, 1.f, 1.f), quadCount));
	//texVertices.push_back(Vertex(glm::vec4(1.f, 1.f, 1.f, 1.f), quadCount));
	//texVertices.push_back(Vertex(glm::vec4(1.f, -1.f, 1.f, 1.f), quadCount));
	//texVertices.push_back(Vertex(glm::vec4(-1.f, -1.f, 1.f, 1.f), quadCount));

	//if (texVertex != nullptr)
	//	delete texVertex;
	//texVertex = new GLVertexBuffer(static_cast<GLsizei>(sizeof(Vertex) * texVertices.size()));
	texVertex->SetData(static_cast<GLsizei>(sizeof(Vertex) * texVertices.size()), texVertices.data());

	uint64_t indexNumber{ texVertices.size() / 4 - 1 };
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 1));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 2));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber + 3));
	texIndices.push_back(static_cast<uint16_t>(4 * indexNumber));
	if (texIndex != nullptr)
		delete texIndex;
	texIndex = new GLIndexBuffer(&texIndices);

	quadCount++;

	//Attributes
	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_4;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(Vertex);
	position_layout.offset = 0;
	position_layout.relative_offset = offsetof(Vertex, position);

	GLAttributeLayout index_layout;
	index_layout.component_type = GLAttributeLayout::Int;
	index_layout.component_dimension = GLAttributeLayout::_1;
	index_layout.normalized = false;
	index_layout.vertex_layout_location = 1;
	index_layout.stride = sizeof(Vertex);
	index_layout.offset = 0;
	index_layout.relative_offset = offsetof(Vertex, index);

	vertexArray.AddVertexBuffer(std::move(*texVertex), sizeof(Vertex), {position_layout, index_layout});
	vertexArray.SetIndexBuffer(std::move(*texIndex));

	//if (uVertex != nullptr)
	//	delete uVertex;
	//uVertex = new VKUniformBuffer<VertexUniform>(&Engine::Instance().GetVKInit(), quadCount);

	//if (uFragment != nullptr)
	//	delete uFragment;
	//uFragment = new VKUniformBuffer<FragmentUniform>(&Engine::Instance().GetVKInit(), quadCount);

	VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexVector.push_back(mat);
	vertexVector.back().color = color_;
	vertexVector.back().isTex = isTex_;
	vertexVector.back().isTexel = isTexel_;

	FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragVector.push_back(tIndex);
}

void GLRenderManager::DeleteWithIndex()
{
	quadCount--;

	if (quadCount == 0)
	{
		texVertices.erase(end(texVertices) - 4, end(texVertices));
		delete texVertex;
		texVertex = nullptr;

		texIndices.erase(end(texIndices) - 6, end(texIndices));
		delete texIndex;
		texIndex = nullptr;

		vertexVector.erase(end(vertexVector) - 1);
		//delete uVertex;
		//uVertex = nullptr;

		fragVector.erase(end(fragVector) - 1);
		//delete uFragment;
		//uFragment = nullptr;

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
