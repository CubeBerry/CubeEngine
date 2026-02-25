//Author: JEYOON YU
//Project: CubeEngine
//File: DXShadowMapContext.cpp
#include "DXShadowMapContext.hpp"
#include "DXCommandListWrapper.hpp"
#include "DXRenderManager.hpp"
#include "Engine.hpp"

void DXShadowMapContext::Initialize()
{
	CreateDepthTexture();

	// Create root signature and pipeline
	// The slot of a root signature version 1.1
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.resize(1);
	rootParameters[0].InitAsConstants(sizeof(PushConstants) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	m_renderManager->CreateRootSignature(m_rootSignature, rootParameters);
	DXHelper::ThrowIfFailed(m_rootSignature->SetName(L"Shadow Map Pass Root Signature"));

	DXAttributeLayout positionLayout{ "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA };

	std::vector<DXGI_FORMAT> rtvFormats = {};
	m_pipeline = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature)
		.SetShaders("../Engine/shaders/hlsl/ShadowMapPass.vert.hlsl", "")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT, true)
		.SetDepthStencil(true, true)
		.SetRenderTargets(rtvFormats)
		.Build();
}

void DXShadowMapContext::OnResize()
{
}

void DXShadowMapContext::Execute(ICommandListWrapper* commandListWrapper)
{
	if (!m_enabled) return;

	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandList->ResourceBarrier(1, &barrier);

	ID3D12PipelineState* initialState = m_pipeline->GetPipelineState().Get();
	commandList->SetPipelineState(initialState);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(m_width), static_cast<float>(m_height), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->OMSetRenderTargets(0, nullptr, FALSE, &m_dsvHandle);
	commandList->ClearDepthStencilView(m_dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_lightViewProjection = CreateLightViewProjection();
	std::vector<DynamicSprite*> sprites = Engine::Instance().GetSpriteManager().GetDynamicSprites();
	for (const auto& sprite : sprites)
	{
		for (auto& subMesh : sprite->GetSubMeshes())
		{
			auto* spriteData = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();
			auto* buffer = subMesh->GetBuffer<BufferWrapper::DXBuffer>();

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Update Constant Buffer
			pushConstants = {
				.decode = spriteData->vertexUniform.decode,
				.localToNDC = m_lightViewProjection * spriteData->vertexUniform.model
			};
			commandList->SetGraphicsRoot32BitConstants(0, sizeof(PushConstants) / 4, &pushConstants, 0);

			// Bind Vertex Buffer & Index Buffer
			D3D12_VERTEX_BUFFER_VIEW vbv = buffer->vertexBuffer->GetView();
			D3D12_INDEX_BUFFER_VIEW ibv = buffer->indexBuffer->GetView();
			commandList->IASetVertexBuffers(0, 1, &vbv);
			commandList->IASetIndexBuffer(&ibv);
			commandList->DrawIndexedInstanced(static_cast<UINT>(spriteData->indices.size()), 1, 0, 0, 0);
		}
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);
}

void DXShadowMapContext::CleanUp()
{
}

void DXShadowMapContext::CreateDepthTexture()
{
	// Create depth texture for shadow mapping
	m_texture.Reset();
	// Use typeless format to be able to create both DSV and SRV with the same resource
	auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32_TYPELESS,
		m_width,
		m_height,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.f;
	clearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&m_texture)
	));
	m_texture->SetName(L"Shadow Depth Texture");

	// Create DSV descriptor for the depth texture
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	m_dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;
	m_renderManager->m_device->CreateDepthStencilView(m_texture.Get(), &dsvDesc, m_dsvHandle);

	// Create SRV descriptor for the depth texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_srvHandle = m_renderManager->AllocateSrvHandles(1);
	m_renderManager->m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHandle.first);
}

glm::mat4 DXShadowMapContext::CreateLightViewProjection() const
{
	// DX12 should use orthZO
	glm::mat4 lightProjection = glm::orthoZO(-m_orthoSize, m_orthoSize, -m_orthoSize, m_orthoSize, m_nearPlane, m_farPlane);
	glm::vec3 lightDirection = glm::normalize(m_lightTarget - m_lightPosition);
	glm::vec3 up{ 0.f, 1.f, 0.f };
	if (std::abs(lightDirection.y) > 0.999f) up = glm::vec3{ 0.f, 0.f, -1.f };
	glm::mat4 lightView = glm::lookAt(m_lightPosition, m_lightTarget, up);
	return lightProjection * lightView;
}

void DXShadowMapContext::DrawImGui()
{
	static bool showDebugFrustum{ true };
	if (ImGui::CollapsingHeader("Shadow Map Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Show Light Frustum", &showDebugFrustum);

		ImGui::DragFloat("Ortho Size", &m_orthoSize, 0.1f, 1.0f, 100.0f);
		ImGui::DragFloat("Near Plane", &m_nearPlane, 0.1f, 0.01f, 100.0f);
		ImGui::DragFloat("Far Plane", &m_farPlane, 0.1f, 1.0f, 1000.0f);

		ImGui::Spacing();
		ImGui::DragFloat3("Light Position", &m_lightPosition.x, 0.1f);
		ImGui::DragFloat3("Light Target", &m_lightTarget.x, 0.1f);
	}

	if (showDebugFrustum)
	{
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		glm::mat4 cameraView = Engine::GetCameraManager().GetViewMatrix();
		glm::mat4 cameraProj = Engine::GetCameraManager().GetProjectionMatrix();

		// Draw light position on the screen for debugging
		glm::vec2 lightScreenPos = Engine::GetRenderManager()->WorldToScreen(m_lightPosition, cameraView, cameraProj);
		if (lightScreenPos.x > 0)
		{
			drawList->AddCircleFilled(ImVec2(lightScreenPos.x, lightScreenPos.y), 8.0f, IM_COL32(255, 255, 0, 255));
			drawList->AddText(ImVec2(lightScreenPos.x + 10, lightScreenPos.y - 10), IM_COL32(255, 255, 0, 255), "Sun");
		}

		// @TODO Study how this code works
		// Draw the light's view frustum by transforming the corners of the NDC cube back to world space and then to screen space
		glm::mat4 invLightVP = glm::inverse(m_lightViewProjection);
		glm::vec3 ndcCorners[8] = {
			// Near Plane (Z = 0)
			{-1.0f, -1.0f, 0.0f}, { 1.0f, -1.0f, 0.0f}, { 1.0f,  1.0f, 0.0f}, {-1.0f,  1.0f, 0.0f},
			// Far Plane (Z = 1)
			{-1.0f, -1.0f, 1.0f}, { 1.0f, -1.0f, 1.0f}, { 1.0f,  1.0f, 1.0f}, {-1.0f,  1.0f, 1.0f}
		};
		ImVec2 screenCorners[8];
		bool valid[8];
		for (int i = 0; i < 8; ++i)
		{
			glm::vec4 worldPos = invLightVP * glm::vec4(ndcCorners[i], 1.0f);
			glm::vec3 worldPos3D = glm::vec3(worldPos) / worldPos.w;

			glm::vec2 screenPos = Engine::GetRenderManager()->WorldToScreen(worldPos3D, cameraView, cameraProj);
			screenCorners[i] = ImVec2(screenPos.x, screenPos.y);
			valid[i] = screenPos.x != -1.0f && screenPos.y != -1.0f;
		}

		auto DrawLine = [&](int index1, int index2)
			{
				if (valid[index1] && valid[index2]) drawList->AddLine(screenCorners[index1], screenCorners[index2], IM_COL32(0, 255, 255, 255), 2.0f);
			};

		// Near
		DrawLine(0, 1); DrawLine(1, 2); DrawLine(2, 3); DrawLine(3, 0);
		// Far
		DrawLine(4, 5); DrawLine(5, 6); DrawLine(6, 7); DrawLine(7, 4);
		// Connect Near and Far
		DrawLine(0, 4); DrawLine(1, 5); DrawLine(2, 6); DrawLine(3, 7);
	}
}
