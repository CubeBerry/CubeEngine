//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.cpp
#include "RenderManager.hpp"

#include <iostream>
#include <fstream>
#include <glm/gtx/transform.hpp>
#include <assimp/postprocess.h>

#include "Engine.hpp"

// Buffer
//BufferWrapper::~BufferWrapper()
//{
//	if (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::DX)
//		dynamic_cast<DXRenderManager*>(Engine::GetRenderManager())->WaitForGPU();
//
//	std::visit([]<typename T>(T & buf)
//	{
//		if constexpr (!std::is_same_v<std::decay_t<T>, std::monostate>)
//		{
//			delete buf.vertexBuffer;
//#ifdef _DEBUG
//			delete buf.normalVertexBuffer;
//#endif
//			delete buf.indexBuffer;
//		}
//	}, buffer);
//
//	std::visit([]<typename T>(T & buf)
//	{
//		if constexpr (!std::is_same_v<std::decay_t<T>, std::monostate>)
//		{
//			delete buf.vertexUniformBuffer;
//			delete buf.fragmentUniformBuffer;
//			if constexpr (requires { buf.materialUniformBuffer; })
//			{
//				delete buf.materialUniformBuffer;
//			}
//		}
//	}, uniformBuffer);
//};

// 2D Mesh Creation
glm::mat4 RenderManager::CreateMesh(std::vector<TwoDimension::Vertex>& quantizedVertices)
{
	// Quantization
	// https://cg.postech.ac.kr/papers/mesh_comp_mobile_conference.pdf
	// Encode
	// 1. The largest x, y, and z bounding cube sizes among all partitions.
	glm::vec2 largestBBoxSize{ 2.f };

	// 2. Calculate (Cx, Cy, Cz), the x, y, and z sizes of the quantized cell.
	// 16, 16
	const float xAxisSteps = static_cast<float>((1 << 16) - 1);
	const float yAxisSteps = static_cast<float>((1 << 16) - 1);
	glm::vec2 C{ largestBBoxSize / glm::vec2{ xAxisSteps, yAxisSteps } };

	glm::ivec2 minQuantizedPos{ INT_MAX };

	std::vector<glm::ivec2> quantizedPositions(4);
	{
		// 3. Quantize all vertex positions.
		glm::ivec2 qp{ static_cast<int32_t>(-1.f / C.x), static_cast<int32_t>(1.f / C.y) };
		quantizedPositions[0] = qp;
		minQuantizedPos = glm::min(minQuantizedPos, qp);
	}
	{
		// 3. Quantize all vertex positions.
		glm::ivec2 qp{ static_cast<int32_t>(1.f / C.x), static_cast<int32_t>(1.f / C.y) };
		quantizedPositions[1] = qp;
		minQuantizedPos = glm::min(minQuantizedPos, qp);
	}
	{
		// 3. Quantize all vertex positions.
		glm::ivec2 qp{ static_cast<int32_t>(1.f / C.x), static_cast<int32_t>(-1.f / C.y) };
		quantizedPositions[2] = qp;
		minQuantizedPos = glm::min(minQuantizedPos, qp);
	}
	{
		// 3. Quantize all vertex positions.
		glm::ivec2 qp{ static_cast<int32_t>(-1.f / C.x), static_cast<int32_t>(-1.f / C.y) };
		quantizedPositions[3] = qp;
		minQuantizedPos = glm::min(minQuantizedPos, qp);
	}

	// 4. For each partition, find the minimum quantized coordinates for the x, y, and z axes, and keep the values as the offsets (Ox, Oy, Oz).
	// Then, subtract (Ox, Oy, Oz) from the quantized coordinates of vertices.
	glm::ivec2 O{ minQuantizedPos };
	for (size_t i = 0; i < 4; ++i)
	{
		quantizedPositions[i] -= O;

		uint32_t packedPosition = (quantizedPositions[i].y << 16) | quantizedPositions[i].x;
		quantizedVertices.emplace_back(TwoDimension::Vertex{ packedPosition });
	}

	// Decode
	glm::mat4 translate{ 1.f };
	glm::mat4 scale{ 1.f };
	translate = glm::translate(translate, glm::vec3{ static_cast<float>(O.x) * C.x, static_cast<float>(O.y) * C.y, 0.f });
	scale = glm::scale(scale, glm::vec3{ C.x, C.y, 1.f });
	glm::mat4 decodeMat = translate * scale;

	return decodeMat;
}

// 3D Mesh Creation
void RenderManager::CreateMesh(
	std::vector<SubMesh>& subMeshes,
	MeshType type, const std::filesystem::path& path, int stacks, int slices,
	glm::vec4 color, float metallic, float roughness)
{
	if (type == MeshType::OBJ)
	{
		//Assimp Model Load
		const aiScene* scene = importer.ReadFile(path.string(),
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_FlipUVs |
			aiProcess_JoinIdenticalVertices |
			aiProcess_TransformUVCoords |
			aiProcess_PreTransformVertices
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << '\n';
			std::exit(EXIT_FAILURE);
		}

		glm::vec3 globalMinPos{ FLT_MAX }, globalMaxPos{ FLT_MIN };
		for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
		{
			aiMesh* mesh = scene->mMeshes[m];
			for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
			{
				aiVector3D vertex = mesh->mVertices[v];
				globalMinPos = glm::min(globalMinPos, glm::vec3{ vertex.x, vertex.y, vertex.z });
				globalMaxPos = glm::max(globalMaxPos, glm::vec3{ vertex.x, vertex.y, vertex.z });
			}
		}

		glm::vec3 center = (globalMinPos + globalMaxPos) / 2.f;
		glm::vec3 size = globalMaxPos - globalMinPos;
		float extent = glm::max(size.x, glm::max(size.y, size.z));
		float unitScale = 1.f / extent;

		ProcessNode(subMeshes, scene->mRootNode, scene, 0, globalMaxPos - globalMinPos, center, unitScale, color, metallic, roughness);
		return;
	}

	SubMesh subMesh;
	subMesh = std::make_unique<BufferWrapper>();
	subMesh->Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::ThreeDimension);
	auto& quantizedVertices = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertices;
#ifdef _DEBUG
	auto& normalVertices = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices;
#endif
	auto& indices = subMesh->GetIndices();

	//Position Vector's w value == 1.f, Direction Vector's w value == 0.f
	std::vector<ThreeDimension::Vertex> vertices;
	switch (type)
	{
	case MeshType::PLANE:
	{
		//Verties
		for (int stack = 0; stack <= stacks; ++stack)
		{
			float row = static_cast<float>(stack) / static_cast<float>(stacks);

			for (int slice = 0; slice <= slices; ++slice)
			{
				float col = static_cast<float>(slice) / static_cast<float>(slices);

				vertices.emplace_back(ThreeDimension::Vertex{
					glm::vec3(col - 0.5f, row - 0.5f, 0.0f),
					glm::vec3(0.0f, 0.0f, 1.0f),
					glm::vec2(col, row)
					});
			}
		}

		//Indices
		BuildIndices(vertices, indices, stacks, slices);
	}
	break;
	case MeshType::CUBE:
	{
		std::vector<ThreeDimension::Vertex> planeVertices;
		std::vector<uint32_t> planeIndices;
		//Vertices
		for (int stack = 0; stack <= stacks; ++stack)
		{
			float row = static_cast<float>(stack) / static_cast<float>(stacks);

			for (int slice = 0; slice <= slices; ++slice)
			{
				float col = static_cast<float>(slice) / static_cast<float>(slices);

				planeVertices.emplace_back(ThreeDimension::Vertex{
					glm::vec3(col - 0.5f, row - 0.5f, 0.0f),
					glm::vec3(0.0f, 0.0f, 1.0f),
					glm::vec2(col, row)
					});
			}
		}

		//Indices
		BuildIndices(planeVertices, planeIndices, stacks, slices);

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
				vertices.emplace_back(ThreeDimension::Vertex{
					RoundDecimal(glm::vec3(transformMat * glm::vec4(plane_vertex.position, 1.f))),
					RoundDecimal(glm::vec3(transformMat * glm::vec4(plane_vertex.normal, 0.f))),
					plane_vertex.uv
					});
			}

			//Indices
			for (const auto index : planeIndices)
			{
				indices.push_back(static_cast<uint32_t>(index + static_cast<int>(planeVertices.size()) * i));
			}
		}
	}
	break;
	case MeshType::SPHERE:
	{
		//Vertices
		const float radius = 0.5f;
		for (int stack = 0; stack <= stacks; ++stack)
		{
			const float row = static_cast<float>(stack) / static_cast<float>(stacks);
			const float beta = PI * (row - 0.5f);
			const float sin_beta = sin(beta);
			const float cos_beta = cos(beta);
			for (int slice = 0; slice <= slices; ++slice)
			{
				const float col = static_cast<float>(slice) / static_cast<float>(slices);
				const float alpha = col * PI * 2.f;
				ThreeDimension::Vertex v;
				v.position = glm::vec3(radius * sin(alpha) * cos_beta, radius * sin_beta, radius * cos(alpha) * cos_beta);
				v.normal = glm::normalize(v.position);
				v.normal /= radius;
				v.uv = glm::vec2(col, row);
				vertices.emplace_back(v);
			}
		}

		//Indices
		BuildIndices(vertices, indices, stacks, slices);
	}
	break;
	case MeshType::TORUS:
	{
		//Vertices
		const float R{ 0.35f };
		const float r{ 0.15f };
		float startAngle = 0.f, endAngle = PI * 2.f;
		for (int i = 0; i <= stacks; ++i)
		{
			float row = static_cast<float>(i) / static_cast<float>(stacks);
			float alpha = startAngle + row * (endAngle - startAngle);
			float sinAlpha = sin(alpha);
			float cosAlpha = cos(alpha);
			for (int j = 0; j <= slices; ++j)
			{
				float col = static_cast<float>(j) / static_cast<float>(slices);
				float beta = 2.0f * col * PI;

				ThreeDimension::Vertex v;
				v.position = glm::vec3{ (R + r * cos(beta)) * sinAlpha,r * sin(-beta),(R + r * cos(beta)) * cosAlpha };
				v.position /= 2 * (R + r);

				glm::vec3 c = { R * sin(alpha),0.f,R * cos(alpha) };
				v.normal = v.position - c;
				v.normal /= r;
				v.uv.x = row;
				v.uv.y = col;

				vertices.emplace_back(v);
			}
		}

		//Indices
		BuildIndices(vertices, indices, stacks, slices);
	}
	break;
	case MeshType::CYLINDER:
	{
		//Vertices
		const float radius = 0.5f;
		//const float height = 1.0f;
		for (int i = 0; i <= stacks; ++i)
		{
			float row = static_cast<float>(i) / static_cast<float>(stacks);
			for (int j = 0; j <= slices; ++j)
			{
				float col = static_cast<float>(j) / static_cast<float>(slices);
				float alpha = col * 2.0f * PI;

				ThreeDimension::Vertex v;
				v.position = glm::vec3{ radius * sin(alpha), row - radius, radius * cos(alpha) };
				v.normal = glm::vec3{ v.position.x / radius, v.position.y, v.position.z / radius };
				v.uv.x = col;
				v.uv.y = row;

				vertices.emplace_back(v);
			}
		}

		//Indices
		BuildIndices(vertices, indices, stacks, slices);

		//Top
		ThreeDimension::Vertex P0, Pi;
		P0.position = glm::vec3{ 0.0f,0.5f,0.0f };
		P0.normal = glm::vec3{ 0.f, 1.f, 0.f };
		P0.uv = glm::vec2{ 0.5f, 0.5f };
		vertices.emplace_back(P0);
		size_t top_cap_index = vertices.size() - 1;
		for (int i = 0; i <= slices; ++i)
		{
			float col = (float)i / slices;
			float alpha = col * 2.0f * PI;

			Pi.position = glm::vec3{ radius * sin(alpha), 0.5f, radius * cos(alpha) };
			Pi.normal = glm::vec3{ 0.f, 1.f, 0.f };
			Pi.uv = glm::vec2{ col, 0.f };
			vertices.emplace_back(Pi);

			if (i > 0)
			{
				indices.push_back(static_cast<uint32_t>(top_cap_index));
				indices.push_back(static_cast<uint32_t>(vertices.size()) - 2);
				indices.push_back(static_cast<uint32_t>(vertices.size()) - 1);
			}
		}

		//Bottom
		P0.position = glm::vec3{ 0.0f,-0.5f,0.0f };
		P0.normal = glm::vec3{ 0.f, -1.f, 0.f };
		P0.uv = glm::vec2{ 0.5f, 0.5f };
		vertices.emplace_back(P0);
		size_t bottom_cap_index = vertices.size() - 1;
		size_t current_index = vertices.size();
		for (int i = 0; i <= slices; ++i)
		{
			float col = static_cast<float>(i) / static_cast<float>(slices);
			float alpha = col * 2.0f * PI;

			Pi.position = glm::vec3{ radius * sin(alpha), -0.5f,radius * cos(alpha) };
			Pi.normal = glm::vec3{ 0.f, -1.f, 0.f };
			Pi.uv = glm::vec2{ col, 0.f };
			vertices.emplace_back(Pi);

			if (i > 0)
			{
				indices.push_back(static_cast<uint32_t>(bottom_cap_index));
				indices.push_back(static_cast<uint32_t>(current_index) + i);
				indices.push_back(static_cast<uint32_t>(current_index) + i - 1);
			}
		}
	}
	break;
	case MeshType::CONE:
	{
		//Vertices
		const float height = 1.0f;
		const float radius = 0.5f;
		for (int i = 0; i <= stacks; ++i)
		{
			float row = static_cast<float>(i) / static_cast<float>(stacks);
			for (int j = 0; j <= slices; ++j)
			{
				float col = static_cast<float>(j) / static_cast<float>(slices);
				float alpha = col * 2.0f * PI;

				ThreeDimension::Vertex v;
				//row == stacks
				v.position = glm::vec3{ radius * (height - row) * sin(alpha),row - radius ,radius * (height - row) * cos(alpha) };
				v.normal = v.position / radius;
				v.uv.x = col;
				v.uv.y = 1 - row;

				vertices.emplace_back(v);
			}
		}

		//Indices
		BuildIndices(vertices, indices, stacks, slices);

		//Bottom
		ThreeDimension::Vertex P0, Pi;
		P0.position = glm::vec3{ 0.0f,-0.5f,0.0f };
		P0.normal = glm::vec3{ 0.f, -1.f, 0.f };
		P0.uv = glm::vec2{ 0.5f, 0.5f };
		vertices.emplace_back(P0);
		size_t bottom_cap_index = vertices.size() - 1;
		size_t current_index = vertices.size();
		for (int i = 0; i <= slices; ++i)
		{
			float col = static_cast<float>(i) / slices;
			float alpha = col * 2.0f * PI;

			Pi.position = glm::vec3{ radius * sin(alpha), -0.5f,radius * cos(alpha) };
			Pi.normal = glm::vec3{ 0.f, -1.f, 0.f };
			Pi.uv = glm::vec2{ col, 0.f };
			vertices.emplace_back(Pi);

			if (i > 0)
			{
				indices.push_back(static_cast<uint32_t>(bottom_cap_index));
				indices.push_back(static_cast<uint32_t>(current_index) + i);
				indices.push_back(static_cast<uint32_t>(current_index) + i - 1);
			}
		}
	}
	break;
	default:
		break;
	}

	glm::vec3 minPos(FLT_MAX), maxPos(FLT_MIN);
	for (auto it = vertices.begin(); it != vertices.end(); ++it)
	{
		minPos = glm::min(minPos, glm::vec3(it->position));
		maxPos = glm::max(maxPos, glm::vec3(it->position));
	}

	for (auto it = vertices.begin(); it != vertices.end(); ++it)
	{
#ifdef _DEBUG
		glm::vec3 start = it->position;
		glm::vec3 end = it->position + it->normal * 0.1f;

		normalVertices.push_back(ThreeDimension::NormalVertex{ start });
		normalVertices.push_back(ThreeDimension::NormalVertex{ end });
#endif
	}

	// Uniform
	auto& vertexUniform = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = Quantize(quantizedVertices, vertices, maxPos - minPos);
	vertexUniform.color = color;

	auto& fragmentUniform = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

	auto& material = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().material;
	material.metallic = metallic;
	material.roughness = roughness;

	// Initialize Buffers
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->InitializeBuffers(*subMesh, indices);

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::QuantizedVertex) * quantizedVertices.size()), quantizedVertices.data());
#ifdef _DEBUG
		subMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
#endif

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::UInt;
		position_layout.component_dimension = GLAttributeLayout::_1;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, position);

		GLAttributeLayout normal_layout;
		normal_layout.component_type = GLAttributeLayout::Float;
		normal_layout.component_dimension = GLAttributeLayout::_3;
		normal_layout.normalized = false;
		normal_layout.vertex_layout_location = 1;
		normal_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		normal_layout.offset = 0;
		normal_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, normal);

		GLAttributeLayout uv_layout;
		uv_layout.component_type = GLAttributeLayout::Float;
		uv_layout.component_dimension = GLAttributeLayout::_2;
		uv_layout.normalized = false;
		uv_layout.vertex_layout_location = 2;
		uv_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		uv_layout.offset = 0;
		uv_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, uv);

		GLAttributeLayout tex_sub_index_layout;
		tex_sub_index_layout.component_type = GLAttributeLayout::Int;
		tex_sub_index_layout.component_dimension = GLAttributeLayout::_1;
		tex_sub_index_layout.normalized = false;
		tex_sub_index_layout.vertex_layout_location = 3;
		tex_sub_index_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		tex_sub_index_layout.offset = 0;
		tex_sub_index_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, texSubIndex);

		subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexArray->AddVertexBuffer(std::move(*subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexArray->SetIndexBuffer(std::move(*subMesh->GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

#ifdef _DEBUG
		GLAttributeLayout normal_position_layout;
		normal_position_layout.component_type = GLAttributeLayout::Float;
		normal_position_layout.component_dimension = GLAttributeLayout::_3;
		normal_position_layout.normalized = false;
		normal_position_layout.vertex_layout_location = 0;
		normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
		normal_position_layout.offset = 0;
		normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

		subMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexArray->AddVertexBuffer(std::move(*subMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout });
#endif
	}

	subMeshes.push_back(std::move(subMesh));
}

void RenderManager::BuildIndices(const std::vector<ThreeDimension::Vertex>& vertices, std::vector<uint32_t>& indices, const int stacks, const int slices)
{
	//Indices
	int i0 = 0, i1 = 0, i2 = 0;
	int i3 = 0, i4 = 0, i5 = 0;

	int stride = slices + 1;
	for (int i = 0; i < stacks; ++i)
	{
		int curr_row = i * stride;
		for (int j = 0; j < slices; ++j)
		{
			/*  You need to compute the indices for the first triangle here */
			i0 = curr_row + j;
			i1 = i0 + 1;
			i2 = i1 + stride;

			/*  Ignore degenerate triangle */
			if (!DegenerateTri(vertices[i0].position, vertices[i1].position, vertices[i2].position))
			{
				/*  Add the indices for the first triangle */
				indices.push_back(static_cast<uint32_t>(i0));
				indices.push_back(static_cast<uint32_t>(i1));
				indices.push_back(static_cast<uint32_t>(i2));
			}

			/*  You need to compute the indices for the second triangle here */
			i3 = i2;
			i4 = i3 - 1;
			i5 = i0;

			/*  Ignore degenerate triangle */
			if (!DegenerateTri(vertices[i3].position, vertices[i4].position, vertices[i5].position))
			{
				indices.push_back(static_cast<uint32_t>(i3));
				indices.push_back(static_cast<uint32_t>(i4));
				indices.push_back(static_cast<uint32_t>(i5));
			}
		}
	}
}

void RenderManager::ProcessNode(
	std::vector<SubMesh>& subMeshes,
	const aiNode* node, const aiScene* scene, int childCount,
	glm::vec3 size, glm::vec3 center, float unitScale,
	glm::vec4 color, float metallic, float roughness)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(subMeshes, mesh, scene, childCount, size, center, unitScale, color, metallic, roughness);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(subMeshes, node->mChildren[i], scene, i, size, center, unitScale, color, metallic, roughness);
	}
}

void RenderManager::ProcessMesh(
	std::vector<SubMesh>& subMeshes,
	const aiMesh* mesh, const aiScene* scene, int childCount,
	glm::vec3 size, glm::vec3 center, float unitScale,
	glm::vec4 color, float metallic, float roughness)
{
	SubMesh subMesh;
	subMesh = std::make_unique<BufferWrapper>();
	subMesh->Initialize(Engine::GetRenderManager()->GetGraphicsMode(), RenderType::ThreeDimension);
	auto& quantizedVertices = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertices;
#ifdef _DEBUG
	auto& normalVertices = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().normalVertices;
#endif
	auto& indices = subMesh->GetIndices();

	// Vertices
	std::vector<ThreeDimension::Vertex> vertices;
	for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
	{
		aiVector3D vertex = mesh->mVertices[v];
		aiVector3D normal = mesh->mNormals[v];
		vertices.emplace_back(ThreeDimension::Vertex{
			glm::vec3(vertex.x, vertex.y, vertex.z),
			glm::vec3(normal.x, normal.y, normal.z),
			mesh->HasTextureCoords(0) ? glm::vec2{ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y } : glm::vec2{ 0.f, 0.f },
			childCount }
			);
	}

	// Indices
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
	{
		aiFace face = mesh->mFaces[f];
		for (unsigned int i = 0; i < face.mNumIndices; i++)
		{
			indices.push_back(static_cast<uint32_t>(face.mIndices[i]));
		}
	}

	// Material
	//aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	//LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

	for (auto it = vertices.begin(); it != vertices.end(); ++it)
	{
		it->position -= center;
		it->position *= glm::vec3(unitScale, unitScale, unitScale);

#ifdef _DEBUG
		glm::vec3 start = it->position;
		glm::vec3 end = it->position + it->normal * 0.1f;

		normalVertices.push_back(ThreeDimension::NormalVertex{ start });
		normalVertices.push_back(ThreeDimension::NormalVertex{ end });
#endif
	}

	// Uniform
	auto& vertexUniform = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = Quantize(quantizedVertices, vertices, size * unitScale);
	vertexUniform.color = color;

	auto& fragmentUniform = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().fragmentUniform;
	fragmentUniform.texIndex = 0;

	auto& material = subMesh->GetClassifiedData<BufferWrapper::BufferData3D>().material;
	material.metallic = metallic;
	material.roughness = roughness;

	// Initialize Buffers
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	renderManager->InitializeBuffers(*subMesh, indices);

	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
	{
		subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::QuantizedVertex) * quantizedVertices.size()), quantizedVertices.data());
#ifdef _DEBUG
		subMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
#endif

		//Attributes
		GLAttributeLayout position_layout;
		position_layout.component_type = GLAttributeLayout::UInt;
		position_layout.component_dimension = GLAttributeLayout::_1;
		position_layout.normalized = false;
		position_layout.vertex_layout_location = 0;
		position_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		position_layout.offset = 0;
		position_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, position);

		GLAttributeLayout normal_layout;
		normal_layout.component_type = GLAttributeLayout::Float;
		normal_layout.component_dimension = GLAttributeLayout::_3;
		normal_layout.normalized = false;
		normal_layout.vertex_layout_location = 1;
		normal_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		normal_layout.offset = 0;
		normal_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, normal);

		GLAttributeLayout uv_layout;
		uv_layout.component_type = GLAttributeLayout::Float;
		uv_layout.component_dimension = GLAttributeLayout::_2;
		uv_layout.normalized = false;
		uv_layout.vertex_layout_location = 2;
		uv_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		uv_layout.offset = 0;
		uv_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, uv);

		GLAttributeLayout tex_sub_index_layout;
		tex_sub_index_layout.component_type = GLAttributeLayout::Int;
		tex_sub_index_layout.component_dimension = GLAttributeLayout::_1;
		tex_sub_index_layout.normalized = false;
		tex_sub_index_layout.vertex_layout_location = 3;
		tex_sub_index_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
		tex_sub_index_layout.offset = 0;
		tex_sub_index_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, texSubIndex);

		subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexArray->AddVertexBuffer(std::move(*subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
		subMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexArray->SetIndexBuffer(std::move(*subMesh->GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));

#ifdef _DEBUG
		GLAttributeLayout normal_position_layout;
		normal_position_layout.component_type = GLAttributeLayout::Float;
		normal_position_layout.component_dimension = GLAttributeLayout::_3;
		normal_position_layout.normalized = false;
		normal_position_layout.vertex_layout_location = 0;
		normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
		normal_position_layout.offset = 0;
		normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

		subMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexArray->AddVertexBuffer(std::move(*subMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout });
#endif
	}

	subMeshes.push_back(std::move(subMesh));
}

//@TODO This function is incomplete.
void RenderManager::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		//bool skip{ false };
		//for (unsigned int j = 0; j < textures_loaded.size(); ++j)
		//{
		//	if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
		//	{
		//		textures.push_back(textures_loaded[j]);
		//		skip = true;
		//		break;
		//	}
		//}

		//if (!skip)
		//{
		//	Texture texture;
		//	texture.id = TextureFromFile(str.C_Str(), directory);
		//	texture.type = typeName;
		//	texture.path = str.C_Str();
		//	textures.push_back(texture);
		//	textures_loaded.push_back(texture);
		//}
	}

	//return textures;
}

// Quantization
// https://cg.postech.ac.kr/papers/mesh_comp_mobile_conference.pdf
glm::mat4 RenderManager::Quantize(
	std::vector<ThreeDimension::QuantizedVertex>& quantizedVertices,
	const std::vector<ThreeDimension::Vertex>& vertices,
	// Encode
	// 1. The largest x, y, and z bounding cube sizes among all partitions.
	glm::vec3 largestBBoxSize)
{
	quantizedVertices.resize(vertices.size());

	// 2. Calculate (Cx, Cy, Cz), the x, y, and z sizes of the quantized cell.
	// 16, 16
	const float xAxisSteps = static_cast<float>((1 << 11) - 1); // 2048 - 1
	const float yAxisSteps = static_cast<float>((1 << 11) - 1); // 2048 - 1
	const float zAxisSteps = static_cast<float>((1 << 10) - 1); // 1024 - 1
	glm::vec3 C{ largestBBoxSize / glm::vec3{ xAxisSteps, yAxisSteps, zAxisSteps } };

	glm::ivec3 minQuantizedPos{ INT_MAX };

	std::vector<glm::ivec3> quantizedPositions(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		// 3. Quantize all vertex positions.
		glm::ivec3 qp{ static_cast<int32_t>(vertices[i].position.x / C.x),
					  static_cast<int32_t>(vertices[i].position.y / C.y),
					  static_cast<int32_t>(vertices[i].position.z / C.z) };
		quantizedPositions[i] = qp;

		minQuantizedPos = glm::min(minQuantizedPos, qp);
	}

	// 4. For each partition, find the minimum quantized coordinates for the x, y, and z axes, and keep the values as the offsets (Ox, Oy, Oz).
	// Then, subtract (Ox, Oy, Oz) from the quantized coordinates of vertices.
	glm::ivec3 O{ minQuantizedPos };
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		quantizedPositions[i] -= O;

		uint32_t packedPosition = (quantizedPositions[i].z << 22) | (quantizedPositions[i].y << 11) | quantizedPositions[i].x;
		quantizedVertices[i].position = packedPosition;
		quantizedVertices[i].normal = vertices[i].normal;
		quantizedVertices[i].uv = vertices[i].uv;
		quantizedVertices[i].texSubIndex = vertices[i].texSubIndex;
	}

	// Decode
	glm::mat4 translate{ 1.f };
	glm::mat4 scale{ 1.f };
	translate = glm::translate(translate, glm::vec3{ C.x * static_cast<float>(O.x), C.y * static_cast<float>(O.y), C.z * static_cast<float>(O.z) });
	scale = glm::scale(scale, C);
	glm::mat4 decodeMat = translate * scale;

	return decodeMat;
}

void RenderManager::RenderingControllerForImGui()
{
	RenderManager* renderManager = Engine::GetRenderManager();
	ImGui::Begin("RenderingController");

	// FidelityFX
	if (renderManager->gMode == GraphicsMode::DX)
	{
		auto currentEffect = m_fidelityFX->GetCurrentEffect();
		bool casUpscaling = (currentEffect == FidelityFX::Effect::CAS_UPSCALING);
		
		// Enable/Disable FidelityFX
		bool ffxEnabled = (currentEffect != FidelityFX::Effect::NONE);
		if (ImGui::Checkbox("Enable FidelityFX", &ffxEnabled))
		{
			FidelityFX::Effect newEffect = ffxEnabled ? FidelityFX::Effect::FSR1 : FidelityFX::Effect::NONE;
			UpdateScalePreset(newEffect, FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY, FidelityFX::CASScalePreset::UltraQuality);
		}
		if (ffxEnabled)
		{
			// Enable FSR1/CAS
			int mode = (currentEffect == FidelityFX::Effect::FSR1) ? 1 : 2;
			if (ImGui::RadioButton("FidelityFX FSR1", &mode, 1))
			{
				UpdateScalePreset(FidelityFX::Effect::FSR1, FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY, FidelityFX::CASScalePreset::UltraQuality);
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("FidelityFX CAS", &mode, 2))
			{
				UpdateScalePreset(FidelityFX::Effect::CAS_SHARPEN_ONLY, FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY, FidelityFX::CASScalePreset::UltraQuality);
			}

			// FSR1
			if (currentEffect == FidelityFX::Effect::FSR1)
			{
				bool enableRCAS = m_fidelityFX->GetEnableRCAS();
				if (ImGui::Checkbox("Enable FidelityFX RCAS", &enableRCAS))
				{
					UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), m_fidelityFX->GetScalePreset());
				}
				if (enableRCAS) ImGui::SliderFloat("Sharpness", &m_fidelityFX->m_sharpness, 0.0f, 1.f);
				if (ImGui::BeginMenu("Upscale Preset"))
				{
					if (ImGui::MenuItem("UltraQuality"))
					{
						UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), m_fidelityFX->GetScalePreset());
					}
					if (ImGui::MenuItem("Quality"))
					{
						UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), m_fidelityFX->GetScalePreset());
					}
					if (ImGui::MenuItem("Balanced"))
					{
						UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), m_fidelityFX->GetScalePreset());
					}
					if (ImGui::MenuItem("Performance"))
					{
						UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), m_fidelityFX->GetScalePreset());
					}
					ImGui::EndMenu();
				}
			}
			// CAS
			else
			{
				ImGui::SliderFloat("Sharpness", &m_fidelityFX->m_sharpness, 0.0f, 1.f);
				if (ImGui::Checkbox("Enable FidelityFX CAS Upscaling", &casUpscaling))
				{
					UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), m_fidelityFX->GetScalePreset());
				}
				if (casUpscaling)
				{
					if (ImGui::BeginMenu("Upscale Preset"))
					{
						if (ImGui::MenuItem("UltraQuality"))
						{
							UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), FidelityFX::CASScalePreset::UltraQuality);
						}
						if (ImGui::MenuItem("Quality"))
						{
							UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), FidelityFX::CASScalePreset::Quality);
						}
						if (ImGui::MenuItem("Balanced"))
						{
							UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), FidelityFX::CASScalePreset::Balanced);
						}
						if (ImGui::MenuItem("Performance"))
						{
							UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), FidelityFX::CASScalePreset::Performance);
						}
						if (ImGui::MenuItem("UltraPerformance"))
						{
							UpdateScalePreset(currentEffect, m_fidelityFX->GetFSR1QualityMode(), FidelityFX::CASScalePreset::UltraPerformance);
						}
						ImGui::EndMenu();
					}
				}
			}
		}
	}
	ImGui::Spacing();
	if (ImGui::Button("FILL", ImVec2(100, 0)))
	{
		renderManager->SetPolygonType(PolygonType::FILL);
	}
	ImGui::SameLine();
	if (ImGui::Button("LINE", ImVec2(100, 0)))
	{
		renderManager->SetPolygonType(PolygonType::LINE);
	}
#ifdef _DEBUG
	ImGui::Spacing();
	ImGui::Checkbox("DrawNormals", &isDrawNormals);
	renderManager->DrawNormals(isDrawNormals);
#endif

	ImGui::End();
}
