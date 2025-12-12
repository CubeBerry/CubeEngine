//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsContext.cpp
#include "DXWorkGraphsContext.hpp"
#include "DXRenderManager.hpp"
#include "Engine.hpp"
#include <d3d12.h>
#include "glm/gtc/matrix_access.hpp"

void DXWorkGraphsContext::CheckWorkGraphsSupport() const
{
	// Check Work Graphs Support
	D3D12_FEATURE_DATA_D3D12_OPTIONS21 options = {};
	if (FAILED(m_renderManager->m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options, sizeof(options)))
		|| (options.WorkGraphsTier == D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED))
	{
		m_renderManager->m_workGraphsEnabled = false;
		Engine::GetLogger().LogDebug(LogCategory::D3D12, "Work Graphs Not Supported");
		OutputDebugStringA("Work Graphs Not Supported\n");
	}
	else
	{
		// Check Shader Model 6.8 Support for Work Graphs
		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_8 };
		if (FAILED(m_renderManager->m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
			|| (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_8))
		{
			m_renderManager->m_workGraphsEnabled = false;
			Engine::GetLogger().LogDebug(LogCategory::D3D12, "Work Graphs, Shader Model 6.8 Not Supported");
			OutputDebugStringA("Work Graphs, Shader Model 6.8 Not Supported\n");
		}
		else
		{
			m_renderManager->m_workGraphsEnabled = true;
			Engine::GetLogger().LogDebug(LogCategory::D3D12, "Work Graphs Enabled");
			OutputDebugStringA("Work Graphs Enabled\n");
		}
	}
}

// DirectX 12 Agility SDK 1.715.0-preview or later is required for Mesh Nodes
void DXWorkGraphsContext::CheckMeshNodesSupport() const
{
	// Work Graphs Support must be enabled first
	if (!m_renderManager->m_workGraphsEnabled)
	{
		m_renderManager->m_meshNodesEnabled = false;
		return;
	}

	// Check Mesh Nodes Support
	D3D12_FEATURE_DATA_D3D12_OPTIONS21 options = {};
	if (FAILED(m_renderManager->m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options, sizeof(options)))
		|| (options.WorkGraphsTier < D3D12_WORK_GRAPHS_TIER_1_1))
	{
		m_renderManager->m_meshNodesEnabled = false;
		Engine::GetLogger().LogDebug(LogCategory::D3D12, "Mesh Nodes Not Supported");
		OutputDebugStringA("Mesh Nodes Not Supported\n");
	}
	else
	{
		// Check Shader Model 6.9 Support for Mesh Nodes
		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_9 };
		if (FAILED(m_renderManager->m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
			|| (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_9))
		{
			m_renderManager->m_meshNodesEnabled = false;
			Engine::GetLogger().LogDebug(LogCategory::D3D12, "Mesh Nodes, Shader Model 6.9 Not Supported");
			OutputDebugStringA("Mesh Nodes, Shader Model 6.9 Not Supported\n");
		}
		else
		{
			m_renderManager->m_meshNodesEnabled = true;
			Engine::GetLogger().LogDebug(LogCategory::D3D12, "Mesh Nodes Enabled");
			OutputDebugStringA("Mesh Nodes Enabled\n");
		}
	}
}

std::array<glm::vec4, 6> ExtractFrustumPlanes(const glm::mat4& viewProj)
{
	std::array<glm::vec4, 6> planes;
	// Left
	planes[0] = glm::row(viewProj, 3) + glm::row(viewProj, 0);
	// Right
	planes[1] = glm::row(viewProj, 3) - glm::row(viewProj, 0);
	// Bottom
	planes[2] = glm::row(viewProj, 3) + glm::row(viewProj, 1);
	// Top
	planes[3] = glm::row(viewProj, 3) - glm::row(viewProj, 1);
	// Near
	planes[4] = glm::row(viewProj, 3) + glm::row(viewProj, 2);
	// Far
	planes[5] = glm::row(viewProj, 3) - glm::row(viewProj, 2);

	// Normalize
	for (auto& plane : planes)
	{
		float length = glm::length(glm::vec3(plane));
		plane /= length;
	}
	return planes;
}

// Work Graphs Frustum Culling
void DXWorkGraphsContext::InitializeWorkGraphs()
{
	m_workGraphsStateObject = std::make_unique<DXWorkGraphsStateObject>(m_renderManager->m_device, "../Engine/shaders/cso/WorkGraphsFrustumCulling.lib.cso", L"WorkGraphsFrustumCulling");

	m_cullingDataBuffer = std::make_unique<DXConstantBuffer<CullingData>>(m_renderManager->m_device, 2);
}

void DXWorkGraphsContext::ExecuteWorkGraphs()
{
	if (!m_renderManager->m_workGraphsEnabled || !m_renderManager->m_meshNodesEnabled) return;

	auto* commandList = m_renderManager->m_commandList.Get();
	auto* globalStaticBuffer = Engine::GetSpriteManager().GetGlobalStaticBuffer();
	if (!globalStaticBuffer) return;

	auto* spriteData = globalStaticBuffer->GetData<BufferWrapper::StaticSprite3D>();
	auto* buffer = globalStaticBuffer->GetBuffer<BufferWrapper::DXBuffer>();
	if (!spriteData || !buffer) return;

	if (spriteData->meshlets.empty()) return;

	// Update Culling Data
	glm::mat4 viewProjection = Engine::GetCameraManager().GetProjectionMatrix() * Engine::GetCameraManager().GetViewMatrix();
	auto planes = ExtractFrustumPlanes(viewProjection);
	CullingData cullingData;
	for (int i = 0; i < 6; ++i) cullingData.frustumPlanes[i] = planes[i];
	cullingData.numMeshlets = static_cast<uint32_t>(spriteData->meshlets.size());
	m_cullingDataBuffer->UpdateConstant(&cullingData, sizeof(CullingData), m_renderManager->m_frameIndex);

	D3D12_SET_PROGRAM_DESC programDesc = m_workGraphsStateObject->GetProgramDesc();
	commandList->SetProgram(&programDesc);

	ID3D12RootSignature* globalRootSignature = m_workGraphsStateObject->GetGlobalRootSignature();
	commandList->SetComputeRootSignature(globalRootSignature);

	ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Mesh Nodes Resource Bindings
	// Root parameter 0: CBV(b0) - CullingData constant buffer
	commandList->SetComputeRootConstantBufferView(0, m_cullingDataBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	// Root parameter 1: CBV(b1) - meshletVisualization constant buffer
	commandList->SetComputeRoot32BitConstant(1, m_renderManager->m_meshletVisualization, 0);

	// Pixel Shader Resource Bindings
	// Root parameter 2: CBV(b2) - DirectionalLight constant buffer
	if (m_renderManager->directionalLightUniformBuffer && !m_renderManager->directionalLightUniforms.empty())
	{
		m_renderManager->directionalLightUniformBuffer->UpdateConstant(m_renderManager->directionalLightUniforms.data(), sizeof(ThreeDimension::DirectionalLightUniform) * m_renderManager->directionalLightUniforms.size(), m_renderManager->m_frameIndex);
		commandList->SetComputeRootConstantBufferView(2, m_renderManager->directionalLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	}
	// Root parameter 3: CBV(b3) - PointLight constant buffer
	if (m_renderManager->pointLightUniformBuffer && !m_renderManager->pointLightUniforms.empty())
	{
		m_renderManager->pointLightUniformBuffer->UpdateConstant(m_renderManager->pointLightUniforms.data(), sizeof(ThreeDimension::PointLightUniform) * m_renderManager->pointLightUniforms.size(), m_renderManager->m_frameIndex);
		commandList->SetComputeRootConstantBufferView(3, m_renderManager->pointLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	}
	// Root parameter 4: CBV(b4) - PushConstants
	m_renderManager->pushConstants.activeDirectionalLight = static_cast<int>(m_renderManager->directionalLightUniforms.size());
	m_renderManager->pushConstants.activePointLight = static_cast<int>(m_renderManager->pointLightUniforms.size());
	commandList->SetComputeRoot32BitConstants(4, 2, &m_renderManager->pushConstants, 0);

	// Common Resource Bindings
	// Root parameter 5 ~ 12
	// SRV bindings:
	// t0: globalUniqueVertices
	// t1: globalMeshlets
	// t2: globalVertexUniforms
	// t3: globalUniqueVertexIndices
	// t4: globalPrimitiveIndices
	// t5: globalMeshletBounds (@TODO: needs to be created)
	// t6: globalFragmentUniforms
	// t7: globalMaterialUniforms
	commandList->SetComputeRootShaderResourceView(5, buffer->uniqueStaticVertexBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(6, buffer->meshletBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(7, spriteData->GetVertexUniformBuffer<DXStructuredBuffer<ThreeDimension::VertexUniform>>()->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(8, buffer->uniqueVertexIndexBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(9, buffer->primitiveIndexBuffer->GetGPUVirtualAddress());
	//commandList->SetComputeRootShaderResourceView(10, buffer->primitiveIndexBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(11, spriteData->GetFragmentUniformBuffer<DXStructuredBuffer<ThreeDimension::FragmentUniform>>()->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(12, spriteData->GetMaterialUniformBuffer<DXStructuredBuffer<ThreeDimension::Material>>()->GetGPUVirtualAddress());

	// Root parameter 13: DescriptorTable(SRV(t0, numDescriptors = unbounded, sapce=1))
	commandList->SetComputeRootDescriptorTable(13, m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	// Root parameter 14: DescriptorTable(SRV(t0, numDescriptors = 3, sapce=2))
	if (m_renderManager->skyboxEnabled && m_renderManager->m_skybox)
	{
		commandList->SetComputeRootDescriptorTable(14, m_renderManager->m_skybox->GetIrradianceMapSrv());
	}

	CullEntryRecord record;
	record.gridSize = (cullingData.numMeshlets + 31) / 32;

	D3D12_DISPATCH_GRAPH_DESC dispatchDesc = {};
	dispatchDesc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
	dispatchDesc.NodeCPUInput.EntrypointIndex = m_workGraphsStateObject->GetEntrypointIndex(L"CullNode");
	dispatchDesc.NodeCPUInput.NumRecords = 1;
	dispatchDesc.NodeCPUInput.pRecords = &record;
	dispatchDesc.NodeCPUInput.RecordStrideInBytes = sizeof(CullEntryRecord);

	commandList->DispatchGraph(&dispatchDesc);
}

void DXWorkGraphsContext::PrintWorkGraphsResults()
{
}

// Simple Work Graphs example
//void DXWorkGraphsContext::InitializeWorkGraphs()
//{
//	m_workGraphsStateObject = std::make_unique<DXWorkGraphsStateObject>(m_renderManager->m_device, "../Engine/shaders/cso/WorkGraphs.cso", L"WorkGraphsTutorial");
//
//	std::vector<uint32_t> initData(1024, 0);
//	// 1 SRV + 1 UAV
//	auto [startCpuHandle, startIndex] = m_renderManager->AllocateSrvHandles(2);
//
//	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle(startCpuHandle, 0, m_renderManager->m_srvDescriptorSize);
//	CD3DX12_CPU_DESCRIPTOR_HANDLE uavCpuHandle(startCpuHandle, 1, m_renderManager->m_srvDescriptorSize);
//
//	m_workGraphsUavCpuHandle = uavCpuHandle;
//	CD3DX12_GPU_DESCRIPTOR_HANDLE startGpuHandle(m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart());
//	m_workGraphsUavGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(startGpuHandle, startIndex + 1, m_renderManager->m_srvDescriptorSize);
//
//	// Create Output Buffer
//	m_workGraphsOutputBuffer = std::make_unique<DXStructuredBuffer<uint32_t>>(
//		m_renderManager->m_device,
//		m_renderManager->m_commandQueue,
//		initData,
//		srvCpuHandle,
//		uavCpuHandle
//	);
//
//	UINT64 bufferSize = sizeof(uint32_t) * initData.size();
//
//	// Create Clear Buffer
//	m_zeroBuffer = DXInitializer::CreateBufferResource(
//		m_renderManager->m_device,
//		bufferSize,
//		D3D12_RESOURCE_FLAG_NONE,
//		D3D12_HEAP_TYPE_UPLOAD,
//		D3D12_RESOURCE_STATE_GENERIC_READ
//	);
//	m_zeroBuffer->SetName(L"Zero Init Buffer");
//
//	void* pData = nullptr;
//	m_zeroBuffer->Map(0, nullptr, &pData);
//	memcpy(pData, initData.data(), bufferSize);
//	m_zeroBuffer->Unmap(0, nullptr);
//
//	// Create Readback Buffer
//	//CD3DX12_HEAP_PROPERTIES readbackHeapProperties(D3D12_HEAP_TYPE_READBACK);
//	//CD3DX12_RESOURCE_DESC readbackBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
//
//	//DXHelper::ThrowIfFailed(m_device->CreateCommittedResource(
//	//	&readbackHeapProperties,
//	//	D3D12_HEAP_FLAG_NONE,
//	//	&readbackBufferDesc,
//	//	D3D12_RESOURCE_STATE_COPY_DEST,
//	//	nullptr,
//	//	IID_PPV_ARGS(&m_workGraphsReadBackBuffer)
//	//));
//	m_workGraphsReadBackBuffer = DXInitializer::CreateBufferResource(
//		m_renderManager->m_device,
//		bufferSize,
//		D3D12_RESOURCE_FLAG_NONE,
//		D3D12_HEAP_TYPE_READBACK,
//		D3D12_RESOURCE_STATE_COPY_DEST
//	);
//	m_workGraphsReadBackBuffer->SetName(L"Work Graphs Readback Buffer");
//}
//
//void DXWorkGraphsContext::ExecuteWorkGraphs()
//{
//	if (!m_renderManager->m_workGraphsEnabled) return;
//
//	D3D12_SET_PROGRAM_DESC programDesc = m_workGraphsStateObject->GetProgramDesc();
//	m_renderManager->m_commandList->SetProgram(&programDesc);
//
//	m_renderManager->m_commandList->SetComputeRootSignature(m_workGraphsStateObject->GetGlobalRootSignature());
//
//	// This is not needed if ExecuteWorkGraphs is called after a BeginRender function's SetDescriptorHeaps
//	ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
//	m_renderManager->m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
//
//	m_renderManager->m_commandList->SetComputeRootDescriptorTable(0, m_workGraphsUavGpuHandle);
//
//	// Clear UAV buffer
//	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
//		m_workGraphsOutputBuffer->GetStructuredBuffer(),
//		D3D12_RESOURCE_STATE_COMMON,
//		D3D12_RESOURCE_STATE_COPY_DEST
//	);
//	m_renderManager->m_commandList->ResourceBarrier(1, &barrier);
//
//	m_renderManager->m_commandList->CopyResource(
//		m_workGraphsOutputBuffer->GetStructuredBuffer(),
//		m_zeroBuffer.Get()
//	);
//
//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
//		m_workGraphsOutputBuffer->GetStructuredBuffer(),
//		D3D12_RESOURCE_STATE_COPY_DEST,
//		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
//	);
//	m_renderManager->m_commandList->ResourceBarrier(1, &barrier);
//
//	// Dispatch
//	struct EntryRecord
//	{
//		uint32_t gridSize;
//		uint32_t recordIndex;
//	};
//	std::vector<EntryRecord> inputData;
//	UINT numRecords = 4;
//	inputData.resize(numRecords);
//	for (UINT recordIndex = 0; recordIndex < numRecords; recordIndex++)
//	{
//		inputData[recordIndex].gridSize = recordIndex + 1;
//		inputData[recordIndex].recordIndex = recordIndex;
//	}
//
//	D3D12_DISPATCH_GRAPH_DESC dispatchGraphDesc = {};
//	dispatchGraphDesc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
//	dispatchGraphDesc.NodeCPUInput.EntrypointIndex = m_workGraphsStateObject->GetEntrypointIndex(L"broadcastNode");
//	dispatchGraphDesc.NodeCPUInput.NumRecords = numRecords;
//	dispatchGraphDesc.NodeCPUInput.pRecords = inputData.data();
//	dispatchGraphDesc.NodeCPUInput.RecordStrideInBytes = sizeof(EntryRecord);
//
//	m_renderManager->m_commandList->DispatchGraph(&dispatchGraphDesc);
//
//	barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_workGraphsOutputBuffer->GetStructuredBuffer());
//	m_renderManager->m_commandList->ResourceBarrier(1, &barrier);
//
//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
//		m_workGraphsOutputBuffer->GetStructuredBuffer(),
//		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
//		D3D12_RESOURCE_STATE_COPY_SOURCE
//	);
//	m_renderManager->m_commandList->ResourceBarrier(1, &barrier);
//
//	m_renderManager->m_commandList->CopyResource(m_workGraphsReadBackBuffer.Get(), m_workGraphsOutputBuffer->GetStructuredBuffer());
//
//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
//		m_workGraphsOutputBuffer->GetStructuredBuffer(),
//		D3D12_RESOURCE_STATE_COPY_SOURCE,
//		D3D12_RESOURCE_STATE_COMMON
//	);
//	m_renderManager->m_commandList->ResourceBarrier(1, &barrier);
//}
//
//void DXWorkGraphsContext::PrintWorkGraphsResults()
//{
//	m_renderManager->WaitForGPU();
//
//	uint32_t* pData{ nullptr };
//	D3D12_RANGE readRange{ 0, 1024 * sizeof(uint32_t) };
//	if (SUCCEEDED(m_workGraphsReadBackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData))))
//	{
//		OutputDebugStringA("=== Work Graph Results ===\n");
//		char msg[256];
//
//		for (int i = 0; i < 4; ++i)
//		{
//			sprintf_s(msg, "SecondNode Acc (UAV[%d]): %u\n", i, pData[i]);
//			OutputDebugStringA(msg);
//		}
//
//		for (int i = 0; i < 4; ++i)
//		{
//			sprintf_s(msg, "ThirdNode Count (UAV[%d]): %u\n", 4 + i, pData[4 + i]);
//			OutputDebugStringA(msg);
//		}
//
//		m_workGraphsReadBackBuffer->Unmap(0, nullptr);
//	}
//}
