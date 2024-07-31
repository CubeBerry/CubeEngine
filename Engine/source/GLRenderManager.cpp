//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLRenderManager.cpp
#include "Engine.hpp"
#include <glm/gtx/transform.hpp>

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
	gl3DShader.LoadShader({ { GLShader::VERTEX, "../Engine/shader/OpenGL3D.vert" }, { GLShader::FRAGMENT, "../Engine/shader/OpenGL3D.frag" } });

	vertexUniform2D = new GLUniformBuffer<TwoDimension::VertexUniform>();
	fragmentUniform2D = new GLUniformBuffer<TwoDimension::FragmentUniform>();
	vertexUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms2D);
	fragmentUniform2D->InitUniform(gl2DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms2D);

	vertexUniform3D = new GLUniformBuffer<ThreeDimension::VertexUniform>();
	fragmentUniform3D = new GLUniformBuffer<ThreeDimension::FragmentUniform>();
	vertexUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms3D);
	fragmentUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms3D);
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

	gl3DShader.Use(true);

	//For Texture Array
	//auto texLocation = glCheck(glGetUniformLocation(gl2DShader.GetProgramHandle(), "tex"));
	//glCheck(glUniform1iv(texLocation, static_cast<GLsizei>(samplers.size()), samplers.data()));

	//if (vertexUniform2D != nullptr)
	//{
	//	vertexUniform2D->UpdateUniform(vertexUniforms2D);
	//}
	//if (fragmentUniform2D != nullptr)
	//{
	//	fragmentUniform2D->UpdateUniform(fragUniforms2D);
	//}

	if (vertexUniform3D != nullptr)
	{
		vertexUniform3D->UpdateUniform(vertexUniforms3D);
	}
	if (fragmentUniform3D != nullptr)
	{
		fragmentUniform3D->UpdateUniform(fragUniforms3D);
	}

	vertexArray.Use(true);

	GLDrawIndexed(vertexArray);

	vertexArray.Use(false);
	gl3DShader.Use(false);

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

void GLRenderManager::LoadMesh(MeshType type, glm::vec4 color, int stacks, int slices)
{
	std::vector<ThreeDimension::Vertex> vertices;
	switch (type)
	{
	case MeshType::PLANE:
	{
		//Verties
		for (int stack = 0; stack <= stacks; ++stack)
		{
			float row = static_cast<float>(stack) / stacks;

			for (int slice = 0; slice <= slices; ++slice)
			{
				float col = static_cast<float>(slice) / slices;

				vertices.push_back(ThreeDimension::Vertex(
					glm::vec4(col - 0.5f, 0.5f - row, 0.0f, 1.0f),
					glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
					glm::vec2(col, row),
					quadCount
				));
			}
		}

		//Indices
		int i0 = 0, i1 = 0, i2 = 0;
		for (int i = 0; i < stacks; ++i)
		{
			for (int j = 0; j < slices; ++j)
			{
				/*  You need to compute the indices for the first triangle here */
				i0 = i * (slices + 1) + j;
				i1 = i0 + 1;
				i2 = i1 + slices + 1;

				/*  Ignore degenerate triangle */
				if (!DegenerateTri(vertices[i0].position, vertices[i2].position, vertices[i1].position))
				{
					/*  Add the indices for the first triangle */
					indices.push_back(static_cast<uint16_t>(i0));
					indices.push_back(static_cast<uint16_t>(i2));
					indices.push_back(static_cast<uint16_t>(i1));
				}

				/*  You need to compute the indices for the second triangle here */
				i1 = i2;
				i2 = i1 - 1;

				/*  Ignore degenerate triangle */
				if (!DegenerateTri(vertices[i0].position, vertices[i2].position, vertices[i1].position))
				{
					indices.push_back(static_cast<uint16_t>(i0));
					indices.push_back(static_cast<uint16_t>(i2));
					indices.push_back(static_cast<uint16_t>(i1));
				}
			}
		}
	}
	break;
	case MeshType::CUBE:
	{
		std::vector<ThreeDimension::Vertex> planeVertices;
		std::vector<uint16_t> planeIndices;
		//Vertices
		for (int stack = 0; stack <= stacks; ++stack)
		{
			float row = static_cast<float>(stack) / stacks;

			for (int slice = 0; slice <= slices; ++slice)
			{
				float col = static_cast<float>(slice) / slices;

				planeVertices.push_back(ThreeDimension::Vertex(
					glm::vec4(col - 0.5f, 0.5f - row, 0.0f, 1.0f),
					glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
					glm::vec2(col, row),
					quadCount
				));
			}
		}

		//Indices
		int i0 = 0, i1 = 0, i2 = 0;
		for (int i = 0; i < stacks; ++i)
		{
			for (int j = 0; j < slices; ++j)
			{
				/*  You need to compute the indices for the first triangle here */
				i0 = i * (slices + 1) + j;
				i1 = i0 + 1;
				i2 = i1 + slices + 1;

				/*  Ignore degenerate triangle */
				if (!DegenerateTri(planeVertices[i0].position, planeVertices[i2].position, planeVertices[i1].position))
				{
					/*  Add the indices for the first triangle */
					planeIndices.push_back(static_cast<uint16_t>(i0));
					planeIndices.push_back(static_cast<uint16_t>(i2));
					planeIndices.push_back(static_cast<uint16_t>(i1));
				}

				/*  You need to compute the indices for the second triangle here */
				i1 = i2;
				i2 = i1 - 1;

				/*  Ignore degenerate triangle */
				if (!DegenerateTri(planeVertices[i0].position, planeVertices[i2].position, planeVertices[i1].position))
				{
					planeIndices.push_back(static_cast<uint16_t>(i0));
					planeIndices.push_back(static_cast<uint16_t>(i2));
					planeIndices.push_back(static_cast<uint16_t>(i1));
				}
			}
		}

		const glm::vec3 translateArray[] = {
			glm::vec3(+0.0f, +0.0f, +0.5f), // Z+
			glm::vec3(+0.0f, +0.0f, -0.5f), // Z-
			glm::vec3(+0.5f, +0.0f, +0.0f), // X+
			glm::vec3(-0.5f, +0.0f, +0.0f), // X-
			glm::vec3(+0.0f, +0.5f, +0.0f), // Y+
			glm::vec3(+0.0f, -0.5f, +0.0f), // Y-
		};

		const glm::vec2 rotateArray[] = {
			glm::vec2(+0.0f, +0.0f),           // Z+
			glm::vec2(+0.0f, (float)+PI),      // Z-
			glm::vec2(+0.0f, (float)+HALF_PI), // X+
			glm::vec2(+0.0f, (float)-HALF_PI), // X-
			glm::vec2((float)-HALF_PI, +0.0f), // Y+
			glm::vec2((float)+HALF_PI, +0.0f)  // Y-
		};

		/*  Transform the plane to 6 positions to form the faces of the cube */
		for (int i = 0; i < 6; ++i)
		{
			const glm::mat4 transformMat = glm::translate(translateArray[i]) *
				glm::rotate(rotateArray[i][YINDEX], glm::vec3{ 0, 1, 0 }) *
				glm::rotate(rotateArray[i][XINDEX], glm::vec3{ 1, 0, 0 });

			for (const auto& plane_vertex : planeVertices)
			{
				vertices.push_back(ThreeDimension::Vertex(
					RoundDecimal(glm::vec4(transformMat * glm::vec4(plane_vertex.position))),
					RoundDecimal(glm::vec4(transformMat * glm::vec4(plane_vertex.normal))),
					plane_vertex.uv,
					quadCount
				));
			}

			//Indices
			for (const auto index : planeIndices)
			{
				indices.push_back(static_cast<uint16_t>(index + static_cast<int>(planeVertices.size()) * i));
			}
		}
	}
	break;
	case MeshType::SPHERE:
	{

	}
	break;
	case MeshType::TORUS:
	{

	}
	break;
	case MeshType::CYLINDER:
	{

	}
	break;
	case MeshType::CONE:
	{

	}
	break;
	}

	vertices3D.insert(vertices3D.end(), vertices.begin(), vertices.end());

	if (quadCount > 0)
		delete vertexBuffer;
	vertexBuffer = new GLVertexBuffer;
	vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::Vertex) * vertices3D.size()), vertices3D.data());

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
	position_layout.stride = sizeof(ThreeDimension::Vertex);
	position_layout.offset = 0;
	position_layout.relative_offset = offsetof(ThreeDimension::Vertex, position);

	GLAttributeLayout normal_layout;
	normal_layout.component_type = GLAttributeLayout::Float;
	normal_layout.component_dimension = GLAttributeLayout::_4;
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

	if (vertexUniform3D != nullptr)
		delete vertexUniform3D;
	vertexUniform3D = new GLUniformBuffer<ThreeDimension::VertexUniform>();
	vertexUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 0, "vUniformMatrix", vertexUniforms3D);

	if (fragmentUniform3D != nullptr)
		delete fragmentUniform3D;
	fragmentUniform3D = new GLUniformBuffer<ThreeDimension::FragmentUniform>();
	fragmentUniform3D->InitUniform(gl3DShader.GetProgramHandle(), 1, "fUniformMatrix", fragUniforms3D);


	ThreeDimension::VertexUniform mat;
	mat.model = glm::mat4(1.f);
	mat.view = glm::mat4(1.f);
	mat.projection = glm::mat4(1.f);
	vertexUniforms3D.push_back(mat);
	vertexUniforms3D.back().color = color;

	ThreeDimension::FragmentUniform tIndex;
	tIndex.texIndex = 0;
	fragUniforms3D.push_back(tIndex);
}
