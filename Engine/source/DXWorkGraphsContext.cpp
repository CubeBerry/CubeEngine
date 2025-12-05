//Author: JEYOON YU
//Project: CubeEngine
//File: DXWorkGraphsContext.cpp
#include "DXWorkGraphsContext.hpp"
#include "DXRenderManager.hpp"
#include "Engine.hpp"
#include <d3d12.h>

void DXWorkGraphsContext::CheckWorkGraphsSupport()
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
void DXWorkGraphsContext::CheckMeshNodesSupport()
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

// Work Graphs Frustum Culling
void DXWorkGraphsContext::InitializeWorkGraphs()
{
}

void DXWorkGraphsContext::ExecuteWorkGraphs()
{
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
