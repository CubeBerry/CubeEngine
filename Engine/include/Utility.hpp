//Author: JEYOON YU
//Project: CubeEngine
//File: Utility.hpp
#pragma once
#include <vector>
#include "Material.hpp"

// Meshlet
struct Meshlet
{
	uint32_t VertexCount;
	uint32_t VertexOffset;
	uint32_t PrimitiveCount;
	uint32_t PrimitiveOffset;
};

namespace Utility
{
	namespace MeshletBuilder
	{
		// Greedy Algorithm
		std::vector<Meshlet> BuildMeshletsGreedy(
			const std::vector<ThreeDimension::Vertex>& vertices,
			// https://gpuopen.com/learn/mesh_shaders/mesh_shaders-optimization_and_best_practices/
			// It differs based on GPU, max 128 vertices and 256 primitives are recommended to Radeon
			uint32_t maxVertices = 128,
			uint32_t maxPrimitives = 256
		);
	}
}
