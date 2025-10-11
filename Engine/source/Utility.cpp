//Author: JEYOON YU
//Project: CubeEngine
//File: Utility.cpp
#include "Utility.hpp"

#include <queue>

std::vector<Meshlet> Utility::MeshletBuilder::BuildMeshletsGreedy(const std::vector<ThreeDimension::Vertex>& vertices, uint32_t maxVertices, uint32_t maxPrimitives)
{
	std::vector<bool> usedVertices(vertices.size(), false);
	std::queue<ThreeDimension::Vertex> vertexQueue;
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		if (usedVertices[i]) continue;
		vertexQueue.push(vertices[i]);
		while (!vertexQueue.empty())
		{
			ThreeDimension::Vertex currentVertex = vertexQueue.front();
			vertexQueue.pop();
		}
	}

	return {};
}
