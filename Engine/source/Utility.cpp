//Author: JEYOON YU
//Project: CubeEngine
//File: Utility.cpp
#include "Utility.hpp"

#include <queue>

void Utility::MeshletBuilder::BuildAdjacency(const std::vector<glm::vec3>& inVertices,
	const std::vector<uint32_t>& inIndices,
	std::vector<Meshlet::Vertex>& outVertices,
	std::vector<Meshlet::Triangle>& outTriangles)
{
	outVertices.resize(inVertices.size());
	for (size_t i = 0; i < inVertices.size(); ++i)
	{
		outVertices[i].position = inVertices[i];
	}

	outTriangles.resize(inIndices.size() / 3);
	for (size_t i = 0; i < outTriangles.size(); ++i)
	{
		outTriangles[i].vertices[0] = inIndices[i * 3 + 0];
		outTriangles[i].vertices[1] = inIndices[i * 3 + 1];
		outTriangles[i].vertices[2] = inIndices[i * 3 + 2];
	}

	for (size_t i = 0; i < outTriangles.size(); ++i)
	{
		uint32_t v0 = outTriangles[i].vertices[0];
		uint32_t v1 = outTriangles[i].vertices[1];
		uint32_t v2 = outTriangles[i].vertices[2];
		
		// Adjacent Triangles
		outVertices[v0].triangles.push_back(i);
		outVertices[v1].triangles.push_back(i);
		outVertices[v2].triangles.push_back(i);
	}
}

// @TODO
void Utility::MeshletBuilder::BuildMeshlet(std::map<uint32_t, uint8_t>& vertexMap,
	std::vector<uint32_t>& tempUniqueVertexIndices,
	std::vector<uint8_t>& tempPrimitiveIndices,
	std::vector<Meshlet::Meshlet>& outMeshlets,
	std::vector<uint32_t>& outUniqueVertexIndices,
	std::vector<uint8_t>& outPrimitiveIndices)
{
	Meshlet::Meshlet meshlet;
	meshlet.vertexCount = static_cast<uint32_t>(tempUniqueVertexIndices.size());
	meshlet.vertexOffset = static_cast<uint32_t>(outUniqueVertexIndices.size());
	meshlet.primitiveCount = static_cast<uint32_t>(tempPrimitiveIndices.size() / 3);
	meshlet.primitiveOffset = static_cast<uint32_t>(outPrimitiveIndices.size());

	outMeshlets.push_back(meshlet);

	outUniqueVertexIndices.insert(outUniqueVertexIndices.end(), tempUniqueVertexIndices.begin(), tempUniqueVertexIndices.end());
	outPrimitiveIndices.insert(outPrimitiveIndices.end(), tempPrimitiveIndices.begin(), tempPrimitiveIndices.end());

	vertexMap.clear();
	tempUniqueVertexIndices.clear();
	tempPrimitiveIndices.clear();
}

void Utility::MeshletBuilder::BuildMeshletsGreedy(const std::vector<uint32_t>& inSortedVertexList,
	std::vector<Meshlet::Vertex>& inVertices,
	std::vector<Meshlet::Triangle>& inTriangles,
	std::vector<Meshlet::Meshlet>& outMeshlets,
	std::vector<uint32_t>& outUniqueVertexIndices,
	std::vector<uint8_t>& outPrimitiveIndices,
	uint32_t maxVertices, uint32_t maxPrimitives)
{
	const uint32_t MAX_VERTS_PER_MESHLET = maxVertices;
	const uint32_t MAX_PRIMS_PER_MESHLET = maxPrimitives;

	for (const auto& sortedVertexIndex : inSortedVertexList)
	{
		if (inVertices[sortedVertexIndex].isUsed) continue;
		std::queue<uint32_t> queue;
		queue.push(sortedVertexIndex);
		inVertices[sortedVertexIndex].isUsed = true;

		// uint32_t == Vertex Index, uint8_t == Unique Vertex Index
		std::map<uint32_t, uint8_t> vertexMap;
		std::vector<uint32_t> tempUniqueVertexIndices;
		std::vector<uint8_t> tempPrimitiveIndices;

		while (!queue.empty())
		{
			uint32_t currentVertexIndex = queue.front();
			queue.pop();

			// @TODO
			if (vertexMap.find(currentVertexIndex) == vertexMap.end())
			{
				uint8_t localIndex = static_cast<uint8_t>(tempUniqueVertexIndices.size());
				vertexMap[currentVertexIndex] = localIndex;
				tempUniqueVertexIndices.push_back(currentVertexIndex);
			}

			for (const uint32_t& triangleIndex : inVertices[currentVertexIndex].triangles)
			{
				if (inTriangles[triangleIndex].isUsed) continue;

				uint32_t uniqueVertexCount{ 0 };
				for (int i = 0; i < 3; ++i)
				{
					uint32_t triangleVertexIndex = inTriangles[triangleIndex].vertices[i];
					if (vertexMap.find(triangleVertexIndex) == vertexMap.end())
					{
						uniqueVertexCount++;
					}
				}

				// @TODO
				//for (const uint32_t& triangleVertexIndex : inTriangles[triangleIndex].vertices)
				//{
				//	if (!inVertices[triangleVertexIndex].isUsed) queue.push(triangleVertexIndex);
				//}

				// Mehslet is full
				if (tempUniqueVertexIndices.size() + uniqueVertexCount > MAX_VERTS_PER_MESHLET)
				{
					if (tempPrimitiveIndices.size() / 3 < MAX_PRIMS_PER_MESHLET)
					{
						// Find triangle in border
						std::vector<uint32_t> borderTriangleIndices;
						for (const auto& vertexIndex : tempUniqueVertexIndices)
						{
							for (const uint32_t& adjacentTriangleIndex : inVertices[vertexIndex].triangles)
							{
								if (!inTriangles[adjacentTriangleIndex].isUsed)
								{
									if (std::find(borderTriangleIndices.begin(), borderTriangleIndices.end(), adjacentTriangleIndex) == borderTriangleIndices.end())
									{
										borderTriangleIndices.push_back(adjacentTriangleIndex);
									}
								}
							}
						}

						// @TODO
						//for (const auto& [vertexIndex, uniqueVertexIndex] : vertexMap)
						//{
						//	for (const uint32_t& adjacentTriangleIndex : inVertices[vertexIndex].triangles)
						//	{
						//		if (!inTriangles[adjacentTriangleIndex].isUsed)
						//		{
						//			if (std::find(borderTriangleIndices.begin(), borderTriangleIndices.end(), adjacentTriangleIndex) == borderTriangleIndices.end())
						//			{
						//				borderTriangleIndices.push_back(adjacentTriangleIndex);
						//			}
						//		}
						//	}
						//}

						// if triangle.vertices in Meshlet
						for (const uint32_t& borderTriangleIndex : borderTriangleIndices)
						{
							bool isAllVerticesInMeshlet{ true };
							for (int i = 0; i < 3; ++i)
							{
								if (vertexMap.find(inTriangles[borderTriangleIndex].vertices[i]) == vertexMap.end())
								{
									isAllVerticesInMeshlet = false;
									break;
								}
							}

							// if triangle.vertices in Meshlet then Meshlet.add(triangle)
							if (isAllVerticesInMeshlet)
							{
								inTriangles[borderTriangleIndex].isUsed = true;
								tempPrimitiveIndices.push_back(vertexMap[inTriangles[borderTriangleIndex].vertices[0]]);
								tempPrimitiveIndices.push_back(vertexMap[inTriangles[borderTriangleIndex].vertices[1]]);
								tempPrimitiveIndices.push_back(vertexMap[inTriangles[borderTriangleIndex].vertices[2]]);

								// @TODO
								//Meshlet::Meshlet meshlet;
								//meshlet.vertexOffset = static_cast<uint32_t>(outUniqueVertexIndices.size());
								//meshlet.vertexCount = static_cast<uint32_t>(tempUniqueVertexIndices.size());
								//meshlet.primitiveOffset = static_cast<uint32_t>(outPrimitiveIndices.size());
								//meshlet.primitiveCount = static_cast<uint32_t>(tempPrimitiveIndices.size() / 3);
								//outMeshlets.push_back(meshlet);
								//outUniqueVertexIndices.insert(outUniqueVertexIndices.end(), tempUniqueVertexIndices.begin(), tempUniqueVertexIndices.end());
								//outPrimitiveIndices.insert(outPrimitiveIndices.end(), tempPrimitiveIndices.begin(), tempPrimitiveIndices.end());
							}
						}
					}

					// @TODO
					BuildMeshlet(vertexMap, tempUniqueVertexIndices, tempPrimitiveIndices, outMeshlets, outUniqueVertexIndices, outPrimitiveIndices);

					while (!queue.empty())
					{
						queue.pop();
					}
					queue.push(currentVertexIndex);
					break;
				}

				// @TODO
				inTriangles[triangleIndex].isUsed = true;
				uint8_t localIndices[3];
				for (int i = 0; i < 3; ++i)
				{
					uint32_t vertexIndex = inTriangles[triangleIndex].vertices[i];
					auto it = vertexMap.find(vertexIndex);
					if (it == vertexMap.end())
					{
						inVertices[vertexIndex].isUsed = true;
						queue.push(vertexIndex);

						uint8_t localIndex = static_cast<uint8_t>(tempUniqueVertexIndices.size());
						vertexMap[vertexIndex] = localIndex;
						tempUniqueVertexIndices.push_back(vertexIndex);
						localIndices[i] = localIndex;
					}
					else localIndices[i] = it->second;
				}
				tempPrimitiveIndices.push_back(localIndices[0]);
				tempPrimitiveIndices.push_back(localIndices[1]);
				tempPrimitiveIndices.push_back(localIndices[2]);

				// @TODO
				//inTriangles[triangleIndex].isUsed = true;
				//tempPrimitiveIndices.push_back(vertexMap[inTriangles[triangleIndex].vertices[0]]);
				//tempPrimitiveIndices.push_back(vertexMap[inTriangles[triangleIndex].vertices[1]]);
				//tempPrimitiveIndices.push_back(vertexMap[inTriangles[triangleIndex].vertices[2]]);

				//Meshlet::Meshlet meshlet;
				//meshlet.vertexOffset = static_cast<uint32_t>(outUniqueVertexIndices.size());
				//meshlet.vertexCount = static_cast<uint32_t>(tempUniqueVertexIndices.size());
				//meshlet.primitiveOffset = static_cast<uint32_t>(outPrimitiveIndices.size());
				//meshlet.primitiveCount = static_cast<uint32_t>(tempPrimitiveIndices.size() / 3);
				//outMeshlets.push_back(meshlet);
				//outUniqueVertexIndices.insert(outUniqueVertexIndices.end(), tempUniqueVertexIndices.begin(), tempUniqueVertexIndices.end());
				//outPrimitiveIndices.insert(outPrimitiveIndices.end(), tempPrimitiveIndices.begin(), tempPrimitiveIndices.end());
			}
		}
		// @TODO
		BuildMeshlet(vertexMap, tempUniqueVertexIndices, tempPrimitiveIndices, outMeshlets, outUniqueVertexIndices, outPrimitiveIndices);
	}
}
