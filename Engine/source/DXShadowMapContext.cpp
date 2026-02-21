//Author: JEYOON YU
//Project: CubeEngine
//File: DXShadowMapContext.cpp
#include "DXShadowMapContext.hpp"
//#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
//#include "DXSkyboxRenderContext.hpp"
//#include "Engine.hpp"

void DXShadowMapContext::Initialize()
{
	//// Copy G-Buffer SRV handles to Main SRV Heap
	//m_gBufferSrvHandle = m_renderManager->AllocateSrvHandles(4);
	//D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	//D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	//m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//// Create root signature and pipeline
	//// The slot of a root signature version 1.1
	//std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	//rootParameters.resize(3);
	//rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[1].InitAsConstants(sizeof(PushConstants) / 4, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	//CD3DX12_DESCRIPTOR_RANGE1 gBufferSrvRange;
	//gBufferSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 1);
	//rootParameters[2].InitAsDescriptorTable(1, &gBufferSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	//m_renderManager->CreateRootSignature(m_rootSignature, rootParameters);
	//DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Local Lighting-Pass Root Signature"));

	//DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	//D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
	//blendDesc.BlendEnable = TRUE;
	//blendDesc.LogicOpEnable = FALSE;
	//blendDesc.SrcBlend = D3D12_BLEND_ONE;
	//blendDesc.DestBlend = D3D12_BLEND_ONE;
	//blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	//blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	//blendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
	//blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

	//m_pipeline = std::make_unique<DXPipeLine>(
	//	m_renderManager->m_device,
	//	m_rootSignature,
	//	std::filesystem::path("../Engine/shaders/hlsl/LocalLightingPass.vert.hlsl"),
	//	std::filesystem::path("../Engine/shaders/hlsl/LocalLightingPass.frag.hlsl"),
	//	std::initializer_list<DXAttributeLayout>{ positionLayout },
	//	D3D12_FILL_MODE_SOLID,
	//	D3D12_CULL_MODE_FRONT,
	//	sampleDesc,
	//	blendDesc,
	//	// Need to turn on CCW
	//	true,
	//	false,
	//	false,
	//	DXGI_FORMAT_R16G16B16A16_FLOAT,
	//	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	//);
}

void DXShadowMapContext::OnResize()
{
}

void DXShadowMapContext::Execute(ICommandListWrapper* commandListWrapper)
{
}

void DXShadowMapContext::CleanUp()
{
}
