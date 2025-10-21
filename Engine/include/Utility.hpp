//Author: JEYOON YU
//Project: CubeEngine
//File: Utility.hpp
#pragma once
#include <vector>
#include <map>
#include "glm/vec3.hpp"

// Meshlet
namespace Meshlet
{
	struct Vertex
	{
		glm::vec3 position;
		bool isUsed{ false };
		// Adjacent Triangles
		std::vector<uint32_t> triangles;
	};

	struct Triangle
	{
		uint32_t vertices[3];
		bool isUsed{ false };
	};

	struct Meshlet
	{
		uint32_t vertexCount{ 0 };
		uint32_t vertexOffset{ 0 };
		uint32_t primitiveCount{ 0 };
		uint32_t primitiveOffset{ 0 };
	};
}

namespace Utility
{
	namespace MeshletBuilder
	{
		void BuildAdjacency(const std::vector<glm::vec3>& inVertices,
			const std::vector<uint32_t>& inIndices,
			std::vector<Meshlet::Vertex>& outVertices,
			std::vector<Meshlet::Triangle>& outTriangles);

		// @TODO
		void BuildMeshlet(std::map<uint32_t, uint32_t>& vertexMap,
			std::vector<uint32_t>& tempUniqueVertexIndices,
			std::vector<uint32_t>& tempPrimitiveIndices,
			std::vector<Meshlet::Meshlet>& outMeshlets,
			std::vector<uint32_t>& outUniqueVertexIndices,
			std::vector<uint32_t>& outPrimitiveIndices);

		// Greedy Algorithm
		void BuildMeshletsGreedy(const std::vector<uint32_t>& inSortedVertexList,
			std::vector<Meshlet::Vertex>& inVertices,
			std::vector<Meshlet::Triangle>& inTriangles,
			std::vector<Meshlet::Meshlet>& outMeshlets,
			std::vector<uint32_t>& outUniqueVertexIndices,
			std::vector<uint32_t>& outPrimitiveIndices,
			// https://gpuopen.com/learn/mesh_shaders/mesh_shaders-optimization_and_best_practices/
			// It differs based on GPU, max 128 vertices and 256 primitives are recommended to Radeon
			uint32_t maxVertices = 128, uint32_t maxPrimitives = 256
		);

		// meshoptimizer Library
		void BuildMeshletsMeshOptimizer(std::vector<glm::vec3>& inVertices,
			const std::vector<uint32_t>& inIndices,
			std::vector<Meshlet::Meshlet>& outMeshlets,
			std::vector<uint32_t>& outUniqueVertexIndices,
			std::vector<uint32_t>& outPrimitiveIndices,
			// https://gpuopen.com/learn/mesh_shaders/mesh_shaders-optimization_and_best_practices/
			// It differs based on GPU, max 128 vertices and 256 primitives are recommended to Radeon
			// MAX_PRIMITIVES_PER_MESHLET should not bigger than X of numthreads. Also max X of numthreads is 128
			uint32_t maxVertices = 64, uint32_t maxPrimitives = 128
		);
	}
}
