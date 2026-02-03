//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.cpp
#include "RenderManager.hpp"
#include "DXRenderManager.hpp"

#include <iostream>
// std::iota
#include <numeric>
#include <glm/gtx/transform.hpp>
#include <assimp/postprocess.h>

#include "Engine.hpp"

// Helper function to populate bone data into vertices
// This function finds the first empty slot in the vertex's bone array and fills it.
void SetVertexBoneData(ThreeDimension::Vertex& vertex, int boneID, float weight)
{
	for (int i = 0; i < ThreeDimension::MAX_BONE_INFLUENCE; ++i)
	{
		if (vertex.boneIDs[i] < 0)
		{
			vertex.boneIDs[i] = boneID;
			vertex.weights[i] = weight;
			return;
		}
	}
	// If more than 4 bones influence this vertex, excess ones are ignored.
}
// Convert Assimp matrix to GLM matrix
glm::mat4 AssimpToGLMMatrix(const aiMatrix4x4& from)
{
	glm::mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

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
		const aiScene* scene = m_importer.ReadFile(path.string(),
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
			std::cerr << "ERROR::ASSIMP:: " << m_importer.GetErrorString() << '\n';
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
	// Both dynamic and static sprite use DynamicSprite3DMesh structure. SpriteManager will handle the difference.
	subMesh = m_meshShaderEnabled ? std::make_unique<BufferWrapper>(SpriteType::DYNAMIC, true) : std::make_unique<BufferWrapper>(SpriteType::DYNAMIC, false);
	
	auto* sprite = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();
	
	auto& quantizedVertices = sprite->vertices;
#ifdef _DEBUG
	auto& normalVertices = sprite->normalVertices;
#endif
	auto& indices = sprite->indices;

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

	// Mesh Shader
	if (m_meshShaderEnabled)
	{
		std::vector<glm::vec3> positions;
		positions.reserve(vertices.size());
		for (const auto& v : vertices)
		{
			positions.push_back(v.position);
		}

		// Greedy Algorithm
		//std::vector<Meshlet::Vertex> meshletVertices;
		//std::vector<Meshlet::Triangle> meshletTriangles;
		//Utility::MeshletBuilder::BuildAdjacency(positions, indices, meshletVertices, meshletTriangles);

		// @TODO Sorting vertices according to the biggest bounding box axis length required
		//std::vector<uint32_t> sortedVertexList(vertices.size());
		//std::iota(sortedVertexList.begin(), sortedVertexList.end(), 0);

		//Utility::MeshletBuilder::BuildMeshletsGreedy(
		//	sortedVertexList,
		//	meshletVertices,
		//	meshletTriangles,
		//	bufferData3D.Meshlets,
		//	bufferData3D.UniqueVertexIndices,
		//	bufferData3D.PrimitiveIndices
		//);

		// meshoptimizer Library
		Utility::MeshletBuilder::BuildMeshletsMeshOptimizer(
			positions,
			indices,
			sprite->meshlets,
			sprite->uniqueVertexIndices,
			sprite->primitiveIndices
		);
	}

	// Uniform
	auto& vertexUniform = sprite->vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = Quantize(quantizedVertices, vertices, maxPos - minPos);
	vertexUniform.color = color;

	auto& fragmentUniform = sprite->fragmentUniform;
	fragmentUniform.texIndex = 0;

	auto& material = sprite->material;
	material.metallic = metallic;
	material.roughness = roughness;

	// Initialize Buffers
	RenderManager* renderManager = Engine::Instance().GetRenderManager();

	// Only dynamic sprite initialize buffers for each sub-mesh
	if (subMesh->GetSpriteType() == SpriteType::DYNAMIC)
	{
		// @TODO Need to divide into Dynamic and Static Meshes
		renderManager->InitializeDynamicBuffers(*subMesh, indices);

		if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
		{
			auto* buffer = subMesh->GetBuffer<BufferWrapper::GLBuffer>();
			buffer->vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::QuantizedVertex) * quantizedVertices.size()), quantizedVertices.data());
#ifdef _DEBUG
			buffer->normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
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

			buffer->vertexArray->AddVertexBuffer(std::move(*buffer->vertexBuffer), sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
			buffer->vertexArray->SetIndexBuffer(std::move(*buffer->indexBuffer));

#ifdef _DEBUG
			GLAttributeLayout normal_position_layout;
			normal_position_layout.component_type = GLAttributeLayout::Float;
			normal_position_layout.component_dimension = GLAttributeLayout::_3;
			normal_position_layout.normalized = false;
			normal_position_layout.vertex_layout_location = 0;
			normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
			normal_position_layout.offset = 0;
			normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

			buffer->normalVertexArray->AddVertexBuffer(std::move(*buffer->normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout });
#endif
		}
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
	subMesh = m_meshShaderEnabled ? std::make_unique<BufferWrapper>(SpriteType::DYNAMIC, true) : std::make_unique<BufferWrapper>(SpriteType::DYNAMIC, false);
	
	auto* sprite = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();

	auto& quantizedVertices = sprite->vertices;
#ifdef _DEBUG
	auto& normalVertices = sprite->normalVertices;
#endif
	auto& indices = sprite->indices;

	// Vertices
	std::vector<ThreeDimension::Vertex> vertices;
	for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
	{
		aiVector3D vertex = mesh->mVertices[v];
		aiVector3D normal = mesh->mNormals[v];

		// Initialize bone IDs and weights to default values
		/*int boneIDs[ThreeDimension::MAX_BONE_INFLUENCE] = { -1,-1,-1,-1 };
		float weights[ThreeDimension::MAX_BONE_INFLUENCE] = { 0.0f,0.0f,0.0f,0.0f };*/


		vertices.emplace_back(ThreeDimension::Vertex{
			glm::vec3(vertex.x, vertex.y, vertex.z),
			glm::vec3(normal.x, normal.y, normal.z),
			mesh->HasTextureCoords(0) ? glm::vec2{ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y } : glm::vec2{ 0.f, 0.f },
			childCount, { -1,-1,-1,-1 }, { 0.0f,0.0f,0.0f,0.0f } }
			);
	}

	// Process Bones (Skeletal Animation Data)
	// We populate the boneInfoMap stored in the DynamicSprite3DMesh
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[i]->mName.C_Str();

		// Check if bone already exists in the map
		if (sprite->boneInfoMap.find(boneName) == sprite->boneInfoMap.end())
		{
			// New bone found, register it
			ThreeDimension::BoneInfo newBoneInfo;
			newBoneInfo.id = sprite->boneCount;
			newBoneInfo.offset = AssimpToGLMMatrix(mesh->mBones[i]->mOffsetMatrix);

			sprite->boneInfoMap[boneName] = newBoneInfo;
			boneID = sprite->boneCount;
			sprite->boneCount++;
		}
		else
		{
			// Bone already exists
			boneID = sprite->boneInfoMap[boneName].id;
		}

		// Assign weights to vertices affected by this bone
		auto weights = mesh->mBones[i]->mWeights;
		int numWeights = mesh->mBones[i]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
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

	// Mesh Shader
	if (m_meshShaderEnabled)
	{
		std::vector<glm::vec3> positions;
		positions.reserve(vertices.size());
		for (const auto& v : vertices)
		{
			positions.push_back(v.position);
		}

		//std::vector<Meshlet::Vertex> meshletVertices;
		//std::vector<Meshlet::Triangle> meshletTriangles;
		//Utility::MeshletBuilder::BuildAdjacency(positions, indices, meshletVertices, meshletTriangles);

		// @TODO Sorting vertices according to the biggest bounding box axis length required
		//std::vector<uint32_t> sortedVertexList(vertices.size());
		//std::iota(sortedVertexList.begin(), sortedVertexList.end(), 0);

		//Utility::MeshletBuilder::BuildMeshletsGreedy(
		//	sortedVertexList,
		//	meshletVertices,
		//	meshletTriangles,
		//	bufferData3D.Meshlets,
		//	bufferData3D.UniqueVertexIndices,
		//	bufferData3D.PrimitiveIndices
		//);

		// meshoptimizer Library
		Utility::MeshletBuilder::BuildMeshletsMeshOptimizer(
			positions,
			indices,
			sprite->meshlets,
			sprite->uniqueVertexIndices,
			sprite->primitiveIndices
		);
	}

	// Uniform
	auto& vertexUniform = sprite->vertexUniform;
	vertexUniform.model = glm::mat4(1.f);
	vertexUniform.view = glm::mat4(1.f);
	vertexUniform.projection = glm::mat4(1.f);
	vertexUniform.decode = Quantize(quantizedVertices, vertices, size * unitScale);
	// @TODO Need to divide into Dynamic and Static Meshes
	//for (uint32_t meshIndex = 0; meshIndex < quantizedVertices.size(); ++meshIndex)
	//{
	//	staticQuantizedVertices.push_back(ThreeDimension::StaticQuantizedVertex{ quantizedVertices[meshIndex], meshIndex });
	//}
	vertexUniform.color = color;

	auto& fragmentUniform = sprite->fragmentUniform;
	fragmentUniform.texIndex = 0;

	auto& material = sprite->material;
	material.metallic = metallic;
	material.roughness = roughness;

	// Initialize Buffers
	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	
	// @TODO Need to divide into Dynamic and Static Meshes
	renderManager->InitializeDynamicBuffers(*subMesh, indices);

	// Only dynamic sprite initialize buffers for each sub-mesh
	if (subMesh->GetSpriteType() == SpriteType::DYNAMIC)
	{
		if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
		{
			auto* buffer = subMesh->GetBuffer<BufferWrapper::GLBuffer>();
			buffer->vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::QuantizedVertex) * quantizedVertices.size()), quantizedVertices.data());
#ifdef _DEBUG
			buffer->normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
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

			// Bone IDs Layout (Location 7)
			// We use GLAttributeLayout::Int for integer attributes (glVertexAttribIFormat)
			GLAttributeLayout bone_id_layout;
			bone_id_layout.component_type = GLAttributeLayout::Int;
			bone_id_layout.component_dimension = GLAttributeLayout::_4;
			bone_id_layout.normalized = false; // Integers are not normalized
			bone_id_layout.vertex_layout_location = 7;
			bone_id_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
			bone_id_layout.offset = 0;
			bone_id_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, boneIDs);

			// Weights Layout (Location 8)
			GLAttributeLayout weight_layout;
			weight_layout.component_type = GLAttributeLayout::Float;
			weight_layout.component_dimension = GLAttributeLayout::_4;
			weight_layout.normalized = false;
			weight_layout.vertex_layout_location = 8;
			weight_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
			weight_layout.offset = 0;
			weight_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, weights);

			buffer->vertexArray->AddVertexBuffer(std::move(*buffer->vertexBuffer), sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout, bone_id_layout, weight_layout });
			buffer->vertexArray->SetIndexBuffer(std::move(*buffer->indexBuffer));

#ifdef _DEBUG
			GLAttributeLayout normal_position_layout;
			normal_position_layout.component_type = GLAttributeLayout::Float;
			normal_position_layout.component_dimension = GLAttributeLayout::_3;
			normal_position_layout.normalized = false;
			normal_position_layout.vertex_layout_location = 0;
			normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
			normal_position_layout.offset = 0;
			normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);

			buffer->normalVertexArray->AddVertexBuffer(std::move(*buffer->normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout });
#endif
		}
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

		// Copy skeletal data directly
		quantizedVertices[i].boneIDs = glm::ivec4(
			vertices[i].boneIDs[0], vertices[i].boneIDs[1],
			vertices[i].boneIDs[2], vertices[i].boneIDs[3]
		);
		quantizedVertices[i].weights = glm::vec4(
			vertices[i].weights[0], vertices[i].weights[1],
			vertices[i].weights[2], vertices[i].weights[3]
		);
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

float RenderManager::CalculatePointLightRadius(const glm::vec3& lightColor, float constant, float linear, float quadratic)
{
	float maxChannel = std::max({ lightColor.r, lightColor.g, lightColor.b });
	float lightThreshold = 4.f / 256.f;
	float c = constant - (maxChannel / lightThreshold);
	float discriminant = linear * linear - 4 * quadratic * c;
	if (discriminant < 0) return 0.f;
	return (-linear + std::sqrt(discriminant)) / (2.f * quadratic);
}

void RenderManager::RenderingControllerForImGui()
{
	// @TODO Might need to make ImGui UI remember state (FSR1/CAS) of FFX effect even if FFX is turned off
	RenderManager* renderManager = Engine::GetRenderManager();
	ImGui::Begin("RenderingController");

	if (renderManager->gMode == GraphicsMode::DX)
	{
		auto* dxRenderManager = dynamic_cast<DXRenderManager*>(renderManager);
		auto* postProcessContext = dxRenderManager->GetPostProcessContext();
		auto* fidelityFX = postProcessContext->GetFidelityFX();

		if (fidelityFX)
		{
			// FidelityFX
			auto currentEffect = fidelityFX->GetCurrentEffect();
			FfxFsr1QualityMode currentFsrMode = fidelityFX->GetFSR1QualityMode();
			FidelityFX::CASScalePreset currentCasScalePreset = fidelityFX->GetSCASScalePreset();
			static FidelityFX::UpscaleEffect lastActiveEffect = FidelityFX::UpscaleEffect::FSR1;
			if (currentEffect != FidelityFX::UpscaleEffect::NONE) lastActiveEffect = currentEffect;

			// Enable/Disable FidelityFX
			bool ffxEnabled = (currentEffect != FidelityFX::UpscaleEffect::NONE);
			int effectMode = (lastActiveEffect == FidelityFX::UpscaleEffect::FSR1) ? 1 : 2;
			if (ImGui::Checkbox("Enable FidelityFX", &ffxEnabled))
			{
				FidelityFX::UpscaleEffect newEffect = ffxEnabled ? lastActiveEffect : FidelityFX::UpscaleEffect::NONE;
				UpdateScalePreset(newEffect, currentFsrMode, currentCasScalePreset);
			}

			if (ffxEnabled)
			{
				// Enable FSR1/CAS
				if (ImGui::RadioButton("FidelityFX FSR1", &effectMode, 1))
				{
					lastActiveEffect = FidelityFX::UpscaleEffect::FSR1;
					UpdateScalePreset(FidelityFX::UpscaleEffect::FSR1, currentFsrMode, currentCasScalePreset);
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("FidelityFX CAS", &effectMode, 2))
				{
					lastActiveEffect = FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY;
					UpdateScalePreset(FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY, currentFsrMode, currentCasScalePreset);
				}

				bool rcasEnabled = fidelityFX->GetEnableRCAS();
				bool casUpscalingEnabled = (currentEffect == FidelityFX::UpscaleEffect::CAS_UPSCALING);
				// FSR1
				if (currentEffect == FidelityFX::UpscaleEffect::FSR1)
				{
					if (ImGui::Checkbox("Enable FidelityFX RCAS", &rcasEnabled))
					{
						fidelityFX->SetEnableRCAS(rcasEnabled);
					}

					const char* fsrLabels[] = { "Ultra Quality", "Quality", "Balanced", "Performance" };
					int currentFsrModeInt = static_cast<int>(currentFsrMode);
					if (ImGui::Combo("Upscale Preset", &currentFsrModeInt, fsrLabels, IM_ARRAYSIZE(fsrLabels)))
					{
						UpdateScalePreset(FidelityFX::UpscaleEffect::FSR1, static_cast<FfxFsr1QualityMode>(currentFsrModeInt), currentCasScalePreset);
					}
				}
				// CAS
				else if (currentEffect == FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY || currentEffect == FidelityFX::UpscaleEffect::CAS_UPSCALING)
				{
					if (ImGui::Checkbox("Enable FidelityFX CAS Upscaling", &casUpscalingEnabled))
					{
						FidelityFX::UpscaleEffect newEffect = casUpscalingEnabled ? FidelityFX::UpscaleEffect::CAS_UPSCALING : FidelityFX::UpscaleEffect::CAS_SHARPEN_ONLY;
						lastActiveEffect = newEffect;
						UpdateScalePreset(newEffect, currentFsrMode, currentCasScalePreset);
					}
					if (casUpscalingEnabled)
					{
						const char* casLabels[] = { "Ultra Quality", "Quality", "Balanced", "Performance", "Ultra Performance" };
						int currentCasPresetInt = static_cast<int>(currentCasScalePreset);
						if (ImGui::Combo("Upscale Preset", &currentCasPresetInt, casLabels, IM_ARRAYSIZE(casLabels)))
						{
							UpdateScalePreset(FidelityFX::UpscaleEffect::CAS_UPSCALING, currentFsrMode, static_cast<FidelityFX::CASScalePreset>(currentCasPresetInt));
						}
					}
				}

				// Slider shows up when both FSR1 & RCAS is enabled or CAS (both only sharpening and sharpening & upscaling) is enabled
				if (!(currentEffect == FidelityFX::UpscaleEffect::FSR1 && !rcasEnabled)) ImGui::SliderFloat("Sharpness", &fidelityFX->m_sharpness, 0.0f, 1.f);
			}
		}

		// Meshlet Visualization
		// Currently only works with DX graphics mode
		ImGui::Spacing();
		bool meshletVisualizationEnabled = (m_meshletVisualization > 0);
		if (ImGui::Checkbox("Meshlet Visualization", &meshletVisualizationEnabled))
		{
			m_meshletVisualization = meshletVisualizationEnabled ? 1 : 0;
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
	ImGui::Checkbox("Normal Vector Visualization", &m_normalVectorVisualization);
#endif

	ImGui::End();
}
