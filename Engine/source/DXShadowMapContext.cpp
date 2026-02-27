//Author: JEYOON YU
//Project: CubeEngine
//File: DXShadowMapContext.cpp
#include "DXShadowMapContext.hpp"

#include <d3dcompiler.h>

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

	std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_pipeline = DXPipeLineBuilder(m_renderManager->m_device, m_rootSignature)
		.SetShaders("../Engine/shaders/hlsl/ShadowMapPass.vert.hlsl", "../Engine/shaders/hlsl/ShadowMapPass.frag.hlsl")
		.SetLayout(std::initializer_list<DXAttributeLayout>{ positionLayout })
		.SetRasterizer(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, true)
		.SetDepthStencil(true, true)
		.SetRenderTargets(rtvFormats)
		.Build();

	// Initialize Compute Pipeline for Convolution Blur
	CD3DX12_DESCRIPTOR_RANGE1 srvRange, uavRange;
	srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
	uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

	rootParameters.resize(3);
	rootParameters[0].InitAsConstantBufferView(0, 0);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange);
	rootParameters[2].InitAsDescriptorTable(1, &uavRange);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSigDesc;
	computeRootSigDesc.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ComPtr<ID3DBlob> signature, error;
	DXHelper::ThrowIfFailed(D3D12SerializeVersionedRootSignature(&computeRootSigDesc, &signature, &error));
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSignature)));
	m_computeRootSignature->SetName(L"Shadow Blur Compute Root Signature");

	ComPtr<ID3DBlob> computeShader, errorMessages;
	HRESULT hr = D3DCompileFromFile(L"../Engine/shaders/hlsl/ConvolutionBlur.compute.hlsl", nullptr, nullptr, "computeMain", "cs_5_1", 0, 0, &computeShader, &errorMessages);
	if (FAILED(hr) && errorMessages) OutputDebugStringA(static_cast<const char*>(errorMessages->GetBufferPointer()));

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = m_computeRootSignature.Get();
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_computePipelineState)));
}

void DXShadowMapContext::OnResize()
{
}

void DXShadowMapContext::Execute(ICommandListWrapper* commandListWrapper)
{
	if (!m_enabled || m_renderManager->directionalLightUniforms.empty()) return;

	DXCommandListWrapper* dxCommandListWrapper = dynamic_cast<DXCommandListWrapper*>(commandListWrapper);
	ID3D12GraphicsCommandList10* commandList = dxCommandListWrapper->GetDXCommandList();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_momentTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	ID3D12PipelineState* initialState = m_pipeline->GetPipelineState().Get();
	commandList->SetPipelineState(initialState);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(m_width), static_cast<float>(m_height), 0.f, 1.f };
	D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->OMSetRenderTargets(1, &m_rtvHandle, FALSE, &m_dsvHandle);
	constexpr float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(m_rtvHandle, clearColor, 0, nullptr);
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

	// Convolution Blur Pass
	UINT descSize = m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_GPU_DESCRIPTOR_HANDLE baseGpuHandle(
		m_renderManager->m_srvHeap->GetGPUDescriptorHandleForHeapStart(),
		static_cast<INT>(m_computeSrvBaseIndex),
		descSize
	);

	CD3DX12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_momentTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_blurredMomentTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(2, barriers);

	ID3D12DescriptorHeap* ppHeaps[] = { m_renderManager->m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetPipelineState(m_computePipelineState.Get());
	commandList->SetComputeRootSignature(m_computeRootSignature.Get());

	// Horizontal Blur Pass
	commandList->SetComputeRootConstantBufferView(0, m_blurParamsBuffer[0]->GetGPUVirtualAddress());

	CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle = baseGpuHandle;
	uavHandle.Offset(3, descSize);

	commandList->SetComputeRootDescriptorTable(1, baseGpuHandle);
	commandList->SetComputeRootDescriptorTable(2, uavHandle);
	commandList->Dispatch((m_width + 127) / 128, m_height, 1);

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_blurredMomentTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_momentTexture.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(2, barriers);

	// Vertical Blur Pass
	commandList->SetComputeRootConstantBufferView(0, m_blurParamsBuffer[1]->GetGPUVirtualAddress());

	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle = baseGpuHandle;
	srvHandle.Offset(2, descSize);
	uavHandle = baseGpuHandle;
	uavHandle.Offset(1, descSize);

	commandList->SetComputeRootDescriptorTable(1, srvHandle);
	commandList->SetComputeRootDescriptorTable(2, uavHandle);
	commandList->Dispatch(m_width, (m_height + 127) / 128, 1);

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_momentTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_blurredMomentTexture.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(2, barriers);
}

void DXShadowMapContext::CleanUp()
{
}

void DXShadowMapContext::CreateDepthTexture()
{
	m_momentTexture.Reset();
	m_depthTexture.Reset();

	// Create moment texture (RTV, SRV)
	// Use typeless format to be able to create both DSV and SRV with the same resource
	auto momentTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		m_width,
		m_height,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	D3D12_CLEAR_VALUE momentClearValue;
	momentClearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	momentClearValue.Color[0] = 1.0f; // z
	momentClearValue.Color[1] = 1.0f; // z^2
	momentClearValue.Color[2] = 1.0f; // z^3
	momentClearValue.Color[3] = 1.0f; // z^4

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&momentTextureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&momentClearValue,
		IID_PPV_ARGS(&m_momentTexture)
	));
	m_momentTexture->SetName(L"Shadow Moment Texture");

	// Create RTV descriptor for the moment texture
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	m_rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	m_renderManager->m_device->CreateRenderTargetView(m_momentTexture.Get(), nullptr, m_rtvHandle);

	// Create SRV descriptor for the depth texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_srvHandle = m_renderManager->AllocateSrvHandles(1);
	m_renderManager->m_device->CreateShaderResourceView(m_momentTexture.Get(), &srvDesc, m_srvHandle.first);

	// Create depth texture for shadow mapping (DSV, SRV)
	// Use typeless format to be able to create both DSV and SRV with the same resource
	auto depthTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32_TYPELESS,
		m_width,
		m_height,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE depthClearValue;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.f;
	depthClearValue.DepthStencil.Stencil = 0;

	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthTextureDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&m_depthTexture)
	));
	m_depthTexture->SetName(L"Shadow Depth Texture");

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
	m_renderManager->m_device->CreateDepthStencilView(m_depthTexture.Get(), &dsvDesc, m_dsvHandle);

	// Create blurred moment texture for convolution blur pass (UAV, SRV)
	auto blurTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		m_width, m_height,
		1, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &blurTextureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_blurredMomentTexture)));
	m_blurredMomentTexture->SetName(L"Blurred Moment Texture");

	// Create SRV, UAV descriptor for the blurred moment texture
	UINT descSize = m_renderManager->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto allocatedHandles = m_renderManager->AllocateSrvHandles(4);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = allocatedHandles.first;
	m_computeSrvBaseIndex = allocatedHandles.second;

	m_srvHandle = { cpuHandle, m_computeSrvBaseIndex };

	srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	// Create SRV and UAV for both moment texture and blurred moment texture
	// moment texture SRV
	m_renderManager->m_device->CreateShaderResourceView(m_momentTexture.Get(), &srvDesc, cpuHandle);
	// moment texture UAV
	cpuHandle.Offset(1, descSize);
	m_renderManager->m_device->CreateUnorderedAccessView(m_momentTexture.Get(), nullptr, &uavDesc, cpuHandle);
	// blurred moment texture SRV
	cpuHandle.Offset(1, descSize);
	m_renderManager->m_device->CreateShaderResourceView(m_blurredMomentTexture.Get(), &srvDesc, cpuHandle);
	// blurred moment texture UAV
	cpuHandle.Offset(1, descSize);
	m_renderManager->m_device->CreateUnorderedAccessView(m_blurredMomentTexture.Get(), nullptr, &uavDesc, cpuHandle);

	// Create CBV and calculate Gaussian weights for convolution blur pass
	UINT cbvSize = (sizeof(BlurParams) + 255) & ~255;
	auto cbvDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(BlurParams));
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

	int w = 5;
	float s = static_cast<float>(w) / 2.0f;

	for (int i = 0; i < 2; ++i)
	{
		DXHelper::ThrowIfFailed(m_renderManager->m_device->CreateCommittedResource(
			&uploadHeap, D3D12_HEAP_FLAG_NONE, &cbvDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_blurParamsBuffer[i])));
		m_blurParamsBuffer[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_blurParamsMapped[i]));

		float sum = 0.f;
		for (int j = -w; j <= w; ++j)
		{
			float weight = std::exp(-0.5f * (static_cast<float>(j) / s) * (static_cast<float>(j) / s));
			m_blurParamsMapped[i]->weights[j + w] = glm::vec4{ weight, 0.f, 0.f, 0.f };
			sum += weight;
		}
		for (int k = 0; k <= 2 * w; ++k) m_blurParamsMapped[i]->weights[k].x /= sum;
		m_blurParamsMapped[i]->blurWidth = w;
		m_blurParamsMapped[i]->isVertical = i;
	}
}

glm::mat4 DXShadowMapContext::CreateLightViewProjection()
{
	// DX12 should use orthZO
	glm::mat4 lightProjection = glm::orthoZO(-m_orthoSize, m_orthoSize, -m_orthoSize, m_orthoSize, m_nearPlane, m_farPlane);
	//glm::mat4 lightProjection = glm::perspectiveZO(glm::radians(90.0f), 1.0f, m_nearPlane, m_farPlane);
	glm::vec3 lightDirection = glm::normalize(m_renderManager->directionalLightUniforms[0].lightDirection);
	float distance = m_farPlane * 0.5f;
	m_lightPosition = m_lightTarget - lightDirection * distance;

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
		ImGui::DragFloat("Near Plane", &m_nearPlane, 0.1f, 0.01f, m_farPlane - 0.01f);
		ImGui::DragFloat("Far Plane", &m_farPlane, 0.1f, m_nearPlane + 0.01f, 1000.0f);
		if (m_nearPlane >= m_farPlane) m_nearPlane = m_farPlane - 0.01f;

		ImGui::Spacing();
		//ImGui::DragFloat3("Light Position", &m_lightPosition.x, 0.1f);
		ImGui::DragFloat3("Light Target", &m_lightTarget.x, 0.1f);
		ImGui::DragFloat("Shadow Bias", &m_shadowBias, 0.0001f, 0.0f, 0.1f, "%.4f");
	}

	if (showDebugFrustum)
	{
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		glm::mat4 cameraView = Engine::GetCameraManager().GetViewMatrix();
		glm::mat4 cameraProj = Engine::GetCameraManager().GetProjectionMatrix();

		// Draw light position and target on the screen for debugging
		glm::vec2 lightScreenPos = Engine::GetRenderManager()->WorldToScreen(m_lightPosition, cameraView, cameraProj);
		glm::vec2 targetScreenPos = Engine::GetRenderManager()->WorldToScreen(m_lightTarget, cameraView, cameraProj);
		if (lightScreenPos.x != -1.0f && targetScreenPos.x != -1.0f)
		{
			// Draw light position
			drawList->AddCircleFilled(ImVec2(lightScreenPos.x, lightScreenPos.y), 8.0f, IM_COL32(255, 255, 0, 255));
			drawList->AddText(ImVec2(lightScreenPos.x + 10, lightScreenPos.y - 10), IM_COL32(255, 255, 0, 255), "Sun");
			// Draw light target
			drawList->AddCircleFilled(ImVec2(targetScreenPos.x, targetScreenPos.y), 4.0f, IM_COL32(255, 0, 0, 255));
			drawList->AddText(ImVec2(targetScreenPos.x + 10, targetScreenPos.y - 10), IM_COL32(255, 0, 0, 255), "Target");
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
