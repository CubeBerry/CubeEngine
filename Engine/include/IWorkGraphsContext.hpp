//Author: JEYOON YU
//Project: CubeEngine
//File: IWorkGraphsContext.hpp
#pragma once

// Work Graphs References:
// https://devblogs.microsoft.com/directx/d3d12-work-graphs/
// https://gpuopen.com/learn/gpu-work-graphs/gpu-work-graphs-intro/
// https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12HelloWorld/src/HelloWorkGraphs
// Windows Developer Mode must be enabled to use Mesh Nodes
// Work Graphs Mesh Nodes References:
// https://devblogs.microsoft.com/directx/d3d12-mesh-nodes-in-work-graphs/
// https://gpuopen.com/learn/work_graphs_mesh_nodes/work_graphs_mesh_nodes-intro/
// https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12HelloWorld/src/HelloMeshNodes
class IWorkGraphsContext
{
public:
	~IWorkGraphsContext() = default;

	virtual void InitializeWorkGraphs() = 0;
	virtual void ExecuteWorkGraphs() = 0;
	virtual void PrintWorkGraphsResults() = 0;
private:
};
