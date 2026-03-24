//Author: JEYOON YU
//Project: CubeEngine
//File: DXLocalLightingContext.cpp
#include "DXLocalLightingContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "DXSkyboxRenderContext.hpp"
#include "Engine.hpp"

void DXLocalLightingContext::Initialize()
{
	// Copy G-Buffer SRV handles to Main SRV Heap
	m_gBufferSrvHandle = m_renderManager->AllocateSrvHandles(4);
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create root signature and pipeline
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.resize(3);
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstants(sizeof(PushConstants) / 4, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	CD3DX12_DESCRIPTOR_RANGE1 gBufferSrvRange;
	gBufferSrvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 1);
	rootParameters[2].InitAsDescriptorTable(1, &gBufferSrvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	m_renderManager->CreateRootSignature(m_rootSignature, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Local Lighting-Pass Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
	blendDesc.BlendEnable = TRUE;
	blendDesc.LogicOpEnable = FALSE;
	blendDesc.SrcBlend = D3D12_BLEND_ONE;
	blendDesc.DestBlend = D3D12_BLEND_ONE;
	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	m_pipeline = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature)
		.SetShaders("../Engine/shaders/hlsl/LocalLightingPass.vert.hlsl", "../Engine/shaders/hlsl/LocalLightingPass.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT, true)
		.SetDepthStencil(false, false)
		.SetRenderTargets(rtvFormats)
		.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.SetBlendMode(blendDesc)
		.Build();

	// Create Unit Sphere
	CreateUnitSphere();
}

void DXLocalLightingContext::OnResize()
{
	// Copy G-Buffer SRV handles to Main SRV Heap
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_gBufferSrvHandle.first;
	D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = m_renderManager->GetGBufferContext()->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CopyDescriptorsSimple(4, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DXLocalLightingContext::Execute(ICommandListWrapper* commandListWrapper)
{
	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	commandList->SetPipelineState(m_pipeline->GetPipelineState().Get());

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderManager->m_renderTarget->GetHDRRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	if (m_renderManager->pointLightUniformBuffer && !m_renderManager->pointLightUniforms.empty())
	{
		m_renderManager->pointLightUniformBuffer->UpdateConstant(m_renderManager->pointLightUniforms.data(), sizeof(ThreeDimension::PointLightUniform) * m_renderManager->pointLightUniforms.size(), m_renderManager->m_frameIndex);
		commandList->SetGraphicsRootConstantBufferView(0, m_renderManager->pointLightUniformBuffer->GetGPUVirtualAddress(m_renderManager->m_frameIndex));
	}

	D3D12_GPU_DESCRIPTOR_HANDLE gBufferGpuHandle = m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	gBufferGpuHandle.ptr += static_cast<UINT64>(m_gBufferSrvHandle.second) * m_renderManager->m_srvDescriptorSize;
	commandList->SetGraphicsRootDescriptorTable(2, gBufferGpuHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_VERTEX_BUFFER_VIEW vbv = m_unitSphereVertexBuffer->GetView();
	D3D12_INDEX_BUFFER_VIEW ibv = m_unitSphereIndexBuffer->GetView();
	commandList->IASetVertexBuffers(0, 1, &vbv);
	commandList->IASetIndexBuffer(&ibv);

	glm::mat4 inverseView = glm::inverse(Engine::GetCameraManager().GetViewMatrix());
	PushConstants pushConstants = {
		.viewProjection = Engine::GetCameraManager().GetProjectionMatrix() * Engine::GetCameraManager().GetViewMatrix(),
		.viewPosition = glm::vec3(
		inverseView[3].x,
		inverseView[3].y,
		inverseView[3].z
		),
		.padding = 1.f,
		.screenSize = { m_renderManager->m_postProcessContext->GetFidelityFX()->GetRenderWidth() ,m_renderManager->m_postProcessContext->GetFidelityFX()->GetRenderHeight() }
	};
	commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstants) / 4, &pushConstants, 0);

	int lightCount = static_cast<int>(m_renderManager->pointLightUniforms.size());
	if (lightCount > 0)
	commandList->DrawIndexedInstanced(m_sphereIndexCount, lightCount, 0, 0, 0);
}

void DXLocalLightingContext::CleanUp()
{
}

void DXLocalLightingContext::CreateUnitSphere()
{
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;

	constexpr int stacks{ 16 };
	constexpr int slices{ 16 };
	constexpr float radius{ 1.f };

	//Vertices
	for (int stack = 0; stack <= stacks; ++stack)
	{
		const float row = static_cast<float>(stack) / static_cast<float>(stacks);
		const float beta = PI * (row - 0.5f);
		const float sin_beta = sin(beta);
		const float cos_beta = cos(beta);
		for (int slice = 0; slice <= slices; ++slice)
		{
			const float col = static_cast<float>(slice) / static_cast<float>(slices);
			const float alpha = col * PI * 2.f;
			vertices.emplace(vertices.end(), radius * sin(alpha) * cos_beta, radius * sin_beta, radius * cos(alpha) * cos_beta);
		}
	}

	//Indices
	int i0 = 0, i1 = 0, i2 = 0;
	int i3 = 0, i4 = 0, i5 = 0;

	int stride = slices + 1;
	for (int i = 0; i < stacks; ++i)
	{
		int curr_row = i * stride;
		for (int j = 0; j < slices; ++j)
		{
			/*  You need to compute the indices for the first triangle here */
			i0 = curr_row + j;
			i1 = i0 + 1;
			i2 = i1 + stride;

			/*  Add the indices for the first triangle */
			indices.push_back(static_cast<uint32_t>(i0));
			indices.push_back(static_cast<uint32_t>(i1));
			indices.push_back(static_cast<uint32_t>(i2));

			/*  You need to compute the indices for the second triangle here */
			i3 = i2;
			i4 = i3 - 1;
			i5 = i0;

			/*  Ignore degenerate triangle */
			indices.push_back(static_cast<uint32_t>(i3));
			indices.push_back(static_cast<uint32_t>(i4));
			indices.push_back(static_cast<uint32_t>(i5));
		}
	}

	m_sphereIndexCount = static_cast<UINT>(indices.size());

	m_unitSphereVertexBuffer = std::make_unique<DXVertexBuffer>(
		m_renderManager->m_device,
		m_renderManager->m_commandQueue,
		static_cast<UINT>(sizeof(glm::vec3)),
		static_cast<UINT>(sizeof(glm::vec3)) * vertices.size(),
		vertices.data()
	);

	m_unitSphereIndexBuffer = std::make_unique<DXIndexBuffer>(
		m_renderManager->m_device,
		m_renderManager->m_commandQueue,
		&indices
	);
}
