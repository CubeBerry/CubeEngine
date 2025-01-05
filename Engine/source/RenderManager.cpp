//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.cpp
#include "RenderManager.hpp"

#include <iostream>
#include <fstream>
#include <glm/gtx/transform.hpp>

#include "Engine.hpp"


void RenderManager::CreateMesh(MeshType type, const std::filesystem::path& path, int stacks, int slices)
{
	//Position Vector's w value == 1.f, Direction Vector's w value == 0.f
	std::vector<ThreeDimension::Vertex> tempVertices;
	std::vector<uint32_t> tempIndices;
	unsigned int verticesCount{ 0 };
	for (unsigned int vertex : verticesPerMesh)
	{
		verticesCount += vertex;
	}
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

				tempVertices.push_back(ThreeDimension::Vertex(
					glm::vec3(col - 0.5f, row - 0.5f, 0.0f),
					glm::vec3(0.0f, 0.0f, 1.0f),
					glm::vec2(col, row),
					quadCount
				));
			}
		}

		//Indices
		BuildIndices(tempVertices, tempIndices, verticesCount, stacks, slices);
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

				planeVertices.push_back(ThreeDimension::Vertex(
					glm::vec3(col - 0.5f, row - 0.5f, 0.0f),
					glm::vec3(0.0f, 0.0f, 1.0f),
					glm::vec2(col, row),
					quadCount
				));
			}
		}

		//Indices
		BuildIndices(planeVertices, planeIndices, 0, stacks, slices);

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
				tempVertices.push_back(ThreeDimension::Vertex(
					RoundDecimal(glm::vec3(transformMat * glm::vec4(plane_vertex.position, 1.f))),
					RoundDecimal(glm::vec3(transformMat * glm::vec4(plane_vertex.normal, 0.f))),
					plane_vertex.uv,
					quadCount
				));
			}

			//Indices
			for (const auto index : planeIndices)
			{
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + (index + static_cast<int>(planeVertices.size()) * i)));
			}
		}
	}
	break;
	case MeshType::SPHERE:
	{
		//Vertices
		const float rad = 0.5f;
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
				v.position = glm::vec3(rad * sin(alpha) * cos_beta, rad * sin_beta, rad * cos(alpha) * cos_beta);
				v.normal = glm::normalize(v.position);
				v.normal /= rad;
				v.uv = glm::vec2(col, row);
				v.index = quadCount;
				tempVertices.push_back(v);
			}
		}

		//Indices
		BuildIndices(tempVertices, tempIndices, verticesCount, stacks, slices);
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
				v.index = quadCount;

				tempVertices.push_back(v);
			}
		}

		//Indices
		BuildIndices(tempVertices, tempIndices, verticesCount, stacks, slices);
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
				v.index = quadCount;

				tempVertices.push_back(v);
			}
		}

		//Indices
		BuildIndices(tempVertices, tempIndices, verticesCount, stacks, slices);

		//Top
		ThreeDimension::Vertex P0, Pi;
		P0.position = glm::vec3{ 0.0f,0.5f,0.0f };
		P0.normal = glm::vec3{ 0.f, 1.f, 0.f };
		P0.uv = glm::vec2{ 0.5f, 0.5f };
		P0.index = quadCount;
		tempVertices.push_back(P0);
		size_t top_cap_index = tempVertices.size() - 1;
		for (int i = 0; i <= slices; ++i)
		{
			float col = (float)i / slices;
			float alpha = col * 2.0f * PI;

			Pi.position = glm::vec3{ radius * sin(alpha), 0.5f, radius * cos(alpha) };
			Pi.normal = glm::vec3{ 0.f, 1.f, 0.f };
			Pi.uv = glm::vec2{ col, 0.f };
			Pi.index = quadCount;
			tempVertices.push_back(Pi);
			//float deltaAlpha{ (2.f * PI) / slices };
			//Pj.position = glm::vec3{ radius * sin(alpha + deltaAlpha),0.5f,radius * cos(alpha + deltaAlpha) };
			//Pj.normal = glm::vec3{ 0.f, 1.f, 0.f };
			//Pj.uv = glm::vec2{ col, 0.f };
			//vertices.push_back(Pj);

			if (i > 0)
			{
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(top_cap_index));
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(tempVertices.size()) - 2);
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(tempVertices.size()) - 1);
			}
		}

		//Bottom
		P0.position = glm::vec3{ 0.0f,-0.5f,0.0f };
		P0.normal = glm::vec3{ 0.f, -1.f, 0.f };
		P0.uv = glm::vec2{ 0.5f, 0.5f };
		P0.index = quadCount;
		tempVertices.push_back(P0);
		size_t bottom_cap_index = tempVertices.size() - 1;
		size_t current_index = tempVertices.size();
		for (int i = 0; i <= slices; ++i)
		{
			float col = static_cast<float>(i) / static_cast<float>(slices);
			float alpha = col * 2.0f * PI;

			Pi.position = glm::vec3{ radius * sin(alpha), -0.5f,radius * cos(alpha) };
			Pi.normal = glm::vec3{ 0.f, -1.f, 0.f };
			Pi.uv = glm::vec2{ col, 0.f };
			Pi.index = quadCount;
			tempVertices.push_back(Pi);
			//float deltaAlpha{ (2.f * PI) / slices };
			//Pj.position = glm::vec3{ radius * sin(alpha + deltaAlpha),-0.5f,radius * cos(alpha + deltaAlpha) };
			//Pj.normal = glm::vec3{ 0.f, -1.f, 0.f };
			//Pj.uv = glm::vec2{ col, 0.f };
			//vertices.push_back(Pj);

			if (i > 0)
			{
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(bottom_cap_index));
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(current_index) + i);
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(current_index) + i - 1);
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
				v.index = quadCount;

				tempVertices.push_back(v);
			}
		}

		//Indices
		BuildIndices(tempVertices, tempIndices, verticesCount, stacks, slices);

		//Bottom
		ThreeDimension::Vertex P0, Pi;
		P0.position = glm::vec3{ 0.0f,-0.5f,0.0f };
		P0.normal = glm::vec3{ 0.f, -1.f, 0.f };
		P0.uv = glm::vec2{ 0.5f, 0.5f };
		P0.index = quadCount;
		tempVertices.push_back(P0);
		size_t bottom_cap_index = tempVertices.size() - 1;
		size_t current_index = tempVertices.size();
		for (int i = 0; i <= slices; ++i)
		{
			float col = static_cast<float>(i) / slices;
			float alpha = col * 2.0f * PI;

			Pi.position = glm::vec3{ radius * sin(alpha), -0.5f,radius * cos(alpha) };
			Pi.normal = glm::vec3{ 0.f, -1.f, 0.f };
			Pi.uv = glm::vec2{ col, 0.f };
			Pi.index = quadCount;
			tempVertices.push_back(Pi);
			//float deltaAlpha{ (2.f * PI) / slices };
			//Pj.position = glm::vec3{ radius * sin(alpha + deltaAlpha),-0.5f,radius * cos(alpha + deltaAlpha) };
			//Pj.normal = glm::vec3{ 0.f, -1.f, 0.f };
			//vertices.push_back(Pj);

			if (i > 0)
			{
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(bottom_cap_index));
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(current_index) + i);
				tempIndices.push_back(verticesCount + static_cast<uint32_t>(current_index) + i - 1);
			}
		}
	}
	break;
	case MeshType::OBJ:
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

		ProcessNode(scene->mRootNode, scene, verticesCount);

		//for (unsigned int m = 0; m < scene->mNumMeshes; m++)
		//{
		//	aiMesh* mesh = scene->mMeshes[m];

		//	//Vertices
		//	for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		//	{
		//		aiVector3D vertex = mesh->mVertices[v];
		//		aiVector3D normal = mesh->mNormals[v];
		//		tempVertices.push_back(ThreeDimension::Vertex(
		//			glm::vec3(vertex.x, vertex.y, vertex.z),
		//			glm::vec3(normal.x, normal.y, normal.z),
		//			mesh->HasTextureCoords(0) ? glm::vec2{ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y } : glm::vec2{ 0.f, 0.f },
		//			quadCount)
		//		);
		//	}

		//	//Indices
		//	for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		//	{
		//		aiFace face = mesh->mFaces[f];
		//		for (unsigned int i = 0; i < face.mNumIndices; i++)
		//		{
		//			tempIndices.push_back(static_cast<uint32_t>(face.mIndices[i]));
		//		}
		//	}
		//}

		if (type != MeshType::OBJ)
		{
			glm::vec3 minPos(FLT_MAX), maxPos(FLT_MIN);
			for (const auto& vertex : tempVertices)
			{
				minPos = glm::min(minPos, glm::vec3(vertex.position));
				maxPos = glm::max(maxPos, glm::vec3(vertex.position));
			}

			glm::vec3 center;
			float unitScale;

			center = (minPos + maxPos) / 2.f;
			glm::vec3 size = maxPos - minPos;
			float extent = glm::max(size.x, glm::max(size.y, size.z));
			unitScale = 1.f / extent;

			for (auto& vertex : tempVertices)
			{
				vertex.position -= center;
				vertex.position *= glm::vec3(unitScale, unitScale, unitScale);
			}

			for (auto& i : tempIndices)
				i += static_cast<uint32_t>(verticesCount);
		}

		//Custom Model Load
		//std::ifstream file(path);
		//if (!file.is_open())
		//	std::cout << "Open File Failed!" << '\n';

		//std::string line;
		//while (std::getline(file, line))
		//{
		//	std::stringstream ss{ line };
		//	std::string prefix;
		//	ss >> prefix;

		//	if (prefix == "v")
		//	{
		//		glm::vec3 temp_vertex;
		//		ss >> temp_vertex.x >> temp_vertex.y >> temp_vertex.z;
		//		tempVertices.push_back(ThreeDimension::Vertex(
		//			glm::vec4(temp_vertex, 1.f),
		//			glm::vec4(0.f, 0.f, 0.f, 1.f),
		//			glm::vec2(0.f, 0.f),
		//			quadCount)
		//		);
		//	}
		//	else if (prefix == "f")
		//	{
		//		std::string data;

		//		for (int i = 0; i < 3; ++i)
		//		{
		//			ss >> data;
		//			std::stringstream d(data);
		//			std::string index;

		//			std::getline(d, index, '/');

		//			tempIndices.push_back(static_cast<uint32_t>(std::stoi(index) - 1));
		//		}
		//	}
		//}
		//file.close();

		//glm::vec3 minPos(FLT_MAX), maxPos(FLT_MIN);
		//for (const auto& vertex : tempVertices)
		//{
		//	minPos = glm::min(minPos, glm::vec3(vertex.position));
		//	maxPos = glm::max(maxPos, glm::vec3(vertex.position));
		//}

		//glm::vec3 center;
		//float unitScale;

		//center = (minPos + maxPos) / 2.f;
		//glm::vec3 size = maxPos - minPos;
		//float extent = glm::max(size.x, glm::max(size.y, size.z));
		//unitScale = 1.f / extent;

		//for (auto& vertex : tempVertices)
		//{
		//	vertex.position -= glm::vec4(center, 0.f);
		//	vertex.position *= glm::vec4(unitScale, unitScale, unitScale, 1.f);
		//}

		//// Calculate vertex normals here
		////allVertexNormals.assign(allVertices.size(), glm::vec3(0.f));
		//for (size_t i = 0; i < tempIndices.size(); i += 3)
		//{
		//	//Find vectors of triangle
		//	glm::vec3 ab = glm::normalize(tempVertices[tempIndices[i + 1]].position - tempVertices[tempIndices[i]].position);
		//	glm::vec3 ac = glm::normalize(tempVertices[tempIndices[i + 2]].position - tempVertices[tempIndices[i]].position);
		//	glm::vec3 bc = glm::normalize(tempVertices[tempIndices[i + 2]].position - tempVertices[tempIndices[i + 1]].position);

		//	//Find normals
		//	//Mean Weighted by Angle
		//	//|cross(A, B)| = |A||B|sin(x), |A||B| is 1 because of normalization
		//	//|cross(A, B)| == |A||B|sin(x)== sin(x)
		//	//glm::length(normal_first) == |cross(A, B)| == sin(x)
		//	//N = normalize(Sigma sin(xi) * Ni)
		//	glm::vec3 normal_first = glm::cross(ab, ac);
		//	glm::vec3 normal_second = glm::cross(ab, bc);
		//	glm::vec3 normal_third = glm::cross(ac, bc);

		//	//Put normals
		//	tempVertices[tempIndices[i]].normal += glm::vec4(normal_first * glm::length(normal_first), 1.f);
		//	tempVertices[tempIndices[i + 1]].normal += glm::vec4(normal_second * glm::length(normal_second), 1.f);
		//	tempVertices[tempIndices[i + 2]].normal += glm::vec4(normal_third * glm::length(normal_third), 1.f);
		//}

		//for (auto& vn : tempVertices)
		//{
		//	//if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::VK)
		//	//	vn.position.y = -vn.position.y;
		//	vn.normal = glm::vec4(glm::normalize(glm::vec3(vn.normal.x, vn.normal.y, vn.normal.z)), 1.f);
		//}

		//for (auto& i : tempIndices)
		//	i += static_cast<uint32_t>(verticesCount);
	}
	break;
	}

#ifdef _DEBUG
	std::vector<ThreeDimension::NormalVertex> tempNormalVertices;
	for (size_t v = 0; v < tempVertices.size(); ++v)
	{
		glm::vec3 start = tempVertices[v].position;
		glm::vec3 end = tempVertices[v].position + tempVertices[v].normal * 0.1f;

		tempNormalVertices.push_back(ThreeDimension::NormalVertex{ start, glm::vec4{1.f}, static_cast<int>(quadCount) });
		tempNormalVertices.push_back(ThreeDimension::NormalVertex{ end, glm::vec4{1.f}, static_cast<int>(quadCount) });
	}

	normalVerticesPerMesh.push_back(static_cast<unsigned int>(tempNormalVertices.size()));
	normalVertices3D.insert(normalVertices3D.end(), tempNormalVertices.begin(), tempNormalVertices.end());
#endif

	if (type != MeshType::OBJ)
	{
		verticesPerMesh.push_back(static_cast<unsigned int>(tempVertices.size()));
		indicesPerMesh.push_back(static_cast<unsigned int>(tempIndices.size()));
		vertices3D.insert(vertices3D.end(), tempVertices.begin(), tempVertices.end());
		indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());
	}
}

void RenderManager::BuildIndices(const std::vector<ThreeDimension::Vertex>& tempVertices, std::vector<uint32_t>& tempIndices, const unsigned int verticesCount, const int stacks, const int slices)
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
			if (!DegenerateTri(tempVertices[i0].position, tempVertices[i1].position, tempVertices[i2].position))
			{
				/*  Add the indices for the first triangle */
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + i0));
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + i1));
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + i2));
			}

			/*  You need to compute the indices for the second triangle here */
			i3 = i2;
			i4 = i3 - 1;
			i5 = i0;

			/*  Ignore degenerate triangle */
			if (!DegenerateTri(tempVertices[i3].position, tempVertices[i4].position, tempVertices[i5].position))
			{
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + i3));
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + i4));
				tempIndices.push_back(static_cast<uint32_t>(verticesCount + i5));
			}
		}
	}
}

void RenderManager::ProcessNode(aiNode* node, const aiScene* scene, unsigned int& verticesCount)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, verticesCount);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, verticesCount);
	}
}

void RenderManager::ProcessMesh(aiMesh* mesh, const aiScene* scene, unsigned int& verticesCount)
{
	std::vector<ThreeDimension::Vertex> tempVertices;
	std::vector<uint32_t> tempIndices;

	//Vertices
	for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
	{
		aiVector3D vertex = mesh->mVertices[v];
		aiVector3D normal = mesh->mNormals[v];
		tempVertices.push_back(ThreeDimension::Vertex(
			glm::vec3(vertex.x, vertex.y, vertex.z),
			glm::vec3(normal.x, normal.y, normal.z),
			mesh->HasTextureCoords(0) ? glm::vec2{ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y } : glm::vec2{ 0.f, 0.f },
			quadCount)
		);
	}

	//Indices
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
	{
		aiFace face = mesh->mFaces[f];
		for (unsigned int i = 0; i < face.mNumIndices; i++)
		{
			tempIndices.push_back(static_cast<uint32_t>(face.mIndices[i]));
		}
	}

	//Material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	}

	glm::vec3 minPos(FLT_MAX), maxPos(FLT_MIN);
	for (const auto& vertex : tempVertices)
	{
		minPos = glm::min(minPos, glm::vec3(vertex.position));
		maxPos = glm::max(maxPos, glm::vec3(vertex.position));
	}

	glm::vec3 center;
	float unitScale;

	center = (minPos + maxPos) / 2.f;
	glm::vec3 size = maxPos - minPos;
	float extent = glm::max(size.x, glm::max(size.y, size.z));
	unitScale = 1.f / extent;

	//for (auto& vertex : tempVertices)
	//{
	//	vertex.position -= center;
	//	vertex.position *= glm::vec3(unitScale, unitScale, unitScale);
	//}

	for (auto& i : tempIndices)
		i += static_cast<uint32_t>(verticesCount);

	verticesPerMesh.push_back(static_cast<unsigned int>(tempVertices.size()));
	indicesPerMesh.push_back(static_cast<unsigned int>(tempIndices.size()));
	vertices3D.insert(vertices3D.end(), tempVertices.begin(), tempVertices.end());
	indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());

	verticesCount += static_cast<unsigned int>(tempVertices.size());
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
