//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.cpp
#include "RenderManager.hpp"

#include <iostream>
#include <fstream>
#include <glm/gtx/transform.hpp>

void RenderManager::CreateMesh(MeshType type, const std::filesystem::path& path, int stacks, int slices)
{
	std::vector<ThreeDimension::Vertex> tempVertices;
	std::vector<uint16_t> tempIndices;
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
			float row = static_cast<float>(stack) / stacks;

			for (int slice = 0; slice <= slices; ++slice)
			{
				float col = static_cast<float>(slice) / slices;

				tempVertices.push_back(ThreeDimension::Vertex(
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
				if (!DegenerateTri(tempVertices[i0].position, tempVertices[i2].position, tempVertices[i1].position))
				{
					/*  Add the indices for the first triangle */
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i0));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i2));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i1));
				}

				/*  You need to compute the indices for the second triangle here */
				i1 = i2;
				i2 = i1 - 1;

				/*  Ignore degenerate triangle */
				if (!DegenerateTri(tempVertices[i0].position, tempVertices[i2].position, tempVertices[i1].position))
				{
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i0));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i2));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i1));
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
				tempVertices.push_back(ThreeDimension::Vertex(
					RoundDecimal(glm::vec4(transformMat * glm::vec4(plane_vertex.position))),
					RoundDecimal(glm::vec4(transformMat * glm::vec4(plane_vertex.normal))),
					plane_vertex.uv,
					quadCount
				));
			}

			//Indices
			for (const auto index : planeIndices)
			{
				tempIndices.push_back(static_cast<uint16_t>(verticesCount + (index + static_cast<int>(planeVertices.size()) * i)));
			}
		}
	}
	break;
	case MeshType::SPHERE:
	{
		//Vertices
		const float rad = 0.5;
		for (int stack = 0; stack <= stacks; ++stack)
		{
			const float row = static_cast<float>(stack) / stacks;
			const float beta = PI * (row - 0.5f);
			const float sin_beta = sin(beta);
			const float cos_beta = cos(beta);
			for (int slice = 0; slice <= slices; ++slice)
			{
				const float col = static_cast<float>(slice) / slices;
				const float alpha = PI * 2.f - col * PI * 2.f;
				ThreeDimension::Vertex v;
				v.position = glm::vec4(rad * sin(alpha) * cos_beta, rad * sin_beta, rad * cos(alpha) * cos_beta, 1.0f);
				v.normal = glm::vec4(glm::normalize(v.position));
				v.uv = glm::vec2(col, row);
				v.index = quadCount;
				tempVertices.push_back(v);
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
				if (!DegenerateTri(tempVertices[i0].position, tempVertices[i2].position, tempVertices[i1].position))
				{
					/*  Add the indices for the first triangle */
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i0));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i2));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i1));
				}

				/*  You need to compute the indices for the second triangle here */
				i1 = i2;
				i2 = i1 - 1;

				/*  Ignore degenerate triangle */
				if (!DegenerateTri(tempVertices[i0].position, tempVertices[i2].position, tempVertices[i1].position))
				{
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i0));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i2));
					tempIndices.push_back(static_cast<uint16_t>(verticesCount + i1));
				}
			}
		}
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
	case MeshType::OBJ:
	{
		std::ifstream file(path);
		if (!file.is_open())
			std::cout << "Open File Failed!" << '\n';

		std::string line;
		while (std::getline(file, line))
		{
			std::stringstream ss{ line };
			std::string prefix;
			ss >> prefix;

			if (prefix == "v")
			{
				glm::vec3 temp_vertex;
				ss >> temp_vertex.x >> temp_vertex.y >> temp_vertex.z;
				tempVertices.push_back(ThreeDimension::Vertex(
					glm::vec4(temp_vertex, 1.f),
					glm::vec4(0.f, 0.f, 0.f, 1.f),
					glm::vec2(0.f, 0.f),
					quadCount)
				);
			}
			else if (prefix == "f")
			{
				std::string data;

				for (int i = 0; i < 3; ++i)
				{
					ss >> data;
					std::stringstream d(data);
					std::string index;

					std::getline(d, index, '/');

					tempIndices.push_back(static_cast<uint16_t>(std::stoi(index) - 1));
				}
			}
		}
		file.close();

		glm::vec3 min(FLT_MAX), max(FLT_MIN);
		for (const auto& vertex : tempVertices)
		{
			min = glm::min(min, glm::vec3(vertex.position));
			max = glm::max(max, glm::vec3(vertex.position));
		}

		glm::vec3 center;
		float unitScale;

		center = (min + max) / 2.f;
		glm::vec3 size = max - min;
		float extent = glm::max(size.x, glm::max(size.y, size.z));
		unitScale = 1.f / extent;

		for (auto& vertex : tempVertices)
		{
			vertex.position -= glm::vec4(center, 0.f);
			vertex.position *= glm::vec4(unitScale, unitScale, unitScale, 1.f);
		}

		// Calculate vertex normals here
		//allVertexNormals.assign(allVertices.size(), glm::vec3(0.f));
		for (size_t i = 0; i < tempIndices.size(); i += 3)
		{
			//Find vectors of triangle
			glm::vec3 ab = glm::normalize(tempVertices[tempIndices[i + 1]].position - tempVertices[tempIndices[i]].position);
			glm::vec3 ac = glm::normalize(tempVertices[tempIndices[i + 2]].position - tempVertices[tempIndices[i]].position);
			glm::vec3 bc = glm::normalize(tempVertices[tempIndices[i + 2]].position - tempVertices[tempIndices[i + 1]].position);

			//Find normals
			//Mean Weighted by Angle
			//|cross(A, B)| = |A||B|sin(x), |A||B| is 1 because of normalization
			//|cross(A, B)| == |A||B|sin(x)== sin(x)
			//glm::length(normal_first) == |cross(A, B)| == sin(x)
			//N = normalize(Sigma sin(xi) * Ni)
			glm::vec3 normal_first = glm::cross(ab, ac);
			glm::vec3 normal_second = glm::cross(ab, bc);
			glm::vec3 normal_third = glm::cross(ac, bc);

			//Put normals
			tempVertices[tempIndices[i]].normal += glm::vec4(normal_first * glm::length(normal_first), 0.f);
			tempVertices[tempIndices[i + 1]].normal += glm::vec4(normal_second * glm::length(normal_second), 0.f);
			tempVertices[tempIndices[i + 2]].normal += glm::vec4(normal_third * glm::length(normal_third), 0.f);
		}

		for (auto& vn : tempVertices)
			vn.normal = glm::vec4(glm::normalize(glm::vec3(vn.normal.x, vn.normal.y, vn.normal.z)), 1.f);

		for (auto& i : tempIndices)
			i += static_cast<uint16_t>(verticesCount);
	}
	break;
	}

	verticesPerMesh.push_back(static_cast<unsigned int>(tempVertices.size()));
	indicesPerMesh.push_back(static_cast<unsigned int>(tempIndices.size()));
	vertices3D.insert(vertices3D.end(), tempVertices.begin(), tempVertices.end());
	indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());
}
