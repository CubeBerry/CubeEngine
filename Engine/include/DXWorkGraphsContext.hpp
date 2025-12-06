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
#include "glm/mat4x4.hpp"

class DXRenderManager;

struct ObjectData
{
	glm::mat4 model;
	glm::mat4 transposeInverseModel;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 decode;
	glm::vec4 color;
	glm::vec3 viewPosition;
	uint32_t meshletOffset;
	uint32_t meshletCount;
};

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

	void CheckWorkGraphsSupport();
	void CheckMeshNodesSupport();

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
};
