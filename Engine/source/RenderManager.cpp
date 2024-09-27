//Author: JEYOON YU
//Project: CubeEngine
//File: RenderManager.cpp
#include "RenderManager.hpp"
#include <glm/gtx/transform.hpp>

void RenderManager::CreateMesh(MeshType type, int stacks, int slices)
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
	}

	verticesPerMesh.push_back(static_cast<unsigned int>(tempVertices.size()));
	indicesPerMesh.push_back(static_cast<unsigned int>(tempIndices.size()));
	vertices3D.insert(vertices3D.end(), tempVertices.begin(), tempVertices.end());
	indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());
}
