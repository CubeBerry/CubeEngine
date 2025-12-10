//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsContext.hpp
#pragma once
#include "IWorkGraphsContext.hpp"
#include "DXWorkGraphsStateObject.hpp"
#include "DXStructuredBuffer.hpp"
#include "BasicComponents/StaticSprite.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

class DXRenderManager;

// Work Graphs References:
// https://devblogs.microsoft.com/directx/d3d12-work-graphs/
// https://gpuopen.com/learn/gpu-work-graphs/gpu-work-graphs-intro/
// https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12HelloWorld/src/HelloWorkGraphs
// Windows Developer Mode must be enabled to use Mesh Nodes
// Work Graphs Mesh Nodes References:
// https://devblogs.microsoft.com/directx/d3d12-mesh-nodes-in-work-graphs/
// https://gpuopen.com/learn/work_graphs_mesh_nodes/work_graphs_mesh_nodes-intro/
// https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12HelloWorld/src/HelloMeshNodes
class DXWorkGraphsContext : public IWorkGraphsContext
{
public:
	DXWorkGraphsContext(DXRenderManager* manager) : m_renderManager(manager) {};

	void CheckWorkGraphsSupport() const;
	void CheckMeshNodesSupport() const;

	void InitializeWorkGraphs() override;
	void ExecuteWorkGraphs() override;
	void PrintWorkGraphsResults() override;
private:
	DXRenderManager* m_renderManager;

	std::unique_ptr<DXWorkGraphsStateObject> m_workGraphsStateObject;
	std::unique_ptr<DXStructuredBuffer<uint32_t>> m_workGraphsOutputBuffer;
	ComPtr<ID3D12Resource> m_workGraphsReadBackBuffer;
	ComPtr<ID3D12Resource> m_zeroBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_workGraphsUavCpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_workGraphsUavGpuHandle;

	struct CullingData
	{
		glm::vec4 frustumPlanes[6];
		uint32_t numMeshlets;
	};
	std::unique_ptr<DXConstantBuffer<CullingData>> m_cullingDataBuffer;

	struct CullEntryRecord
	{
		uint32_t gridSize;
	};
};
