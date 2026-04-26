//Author: DOYEONG LEE
//Project: CubeEngine
//File: CameraManager.cpp
#include "CameraManager.hpp"
#include "Engine.hpp"

CameraManager::~CameraManager()
{
	Engine::GetLogger().LogDebug(LogCategory::Engine, "Camera Manager Deleted");
}

Camera* CameraManager::AddCamera(CameraType type, std::string name)
{
	auto newCamera = std::make_unique<Camera>();
	newCamera->SetCameraType(type);

	if (name.empty() == true)
	{
		name = "Camera " + std::to_string(maxCameraIndex);
	}
	maxCameraIndex++;
	newCamera->SetName(name);
	
	glm::vec2 wSize = Engine::GetWindow().GetWindowSize();
	newCamera->SetViewSize(static_cast<int>(wSize.x), static_cast<int>(wSize.y));

	cameras.push_back(std::move(newCamera));

	return cameras.back().get();
}

void CameraManager::DeleteCamera(int index)
{
	if (index >= 0 && index < cameras.size())
	{
		cameras.erase(cameras.begin() + index);

		if (mainCameraIndex >= cameras.size())
		{
			mainCameraIndex = static_cast<int>(cameras.size()) - 1;
		}

		if (selectedCameraIndex >= cameras.size())
		{
			selectedCameraIndex = static_cast<int>(cameras.size()) - 1;
		}
	}
}

void CameraManager::SetMainCamera(int index)
{
	if (index >= 0 && index < cameras.size())
	{
		mainCameraIndex = index;
	}
}

Camera* CameraManager::GetCamera(int index)
{
	if (index >= 0 && index < cameras.size())
	{
		return cameras[index].get();
	}
	return nullptr;
}

size_t CameraManager::GetCameraCount() const
{
	return cameras.size();
}

void CameraManager::Init(glm::vec2 viewSize, CameraType type, float zoom, float angle)
{
	AddCamera(type, "Main Camera");
	Camera* mainCam = GetCamera(0);

	if (mainCam != nullptr)
	{
		mainCam->SetViewSize(static_cast<int>(viewSize.x), static_cast<int>(viewSize.y));
		mainCam->SetZoom(zoom);
		mainCam->Rotate2D(angle);
	}

	switch (type)
	{
	case CameraType::TwoDimension:
		break;
	case CameraType::ThreeDimension:
		break;
	default:
		break;
	}
	Engine::GetLogger().LogDebug(LogCategory::Engine, "Camera Manager Initialized");
}

void CameraManager::Update()
{
	for (auto& cam : cameras)
	{
		if (cam->GetIsActive() == true)
		{
			cam->Update();
		}
	}
}

void CameraManager::Reset()
{
	for (auto& cam : cameras)
	{
		cam->Reset();
	}
	maxCameraIndex = 0;
	mainCameraIndex = 0;
	selectedCameraIndex = 0;
	Engine::GetLogger().LogDebug(LogCategory::Engine, "Camera Manager Reset");
}

void CameraManager::DeleteAllCameras()
{
	if(cameras.empty() == true)
	{
		return;
	}

	cameras.clear();
	maxCameraIndex = 0;
	mainCameraIndex = 0;
	selectedCameraIndex = 0;
	Engine::GetLogger().LogDebug(LogCategory::Engine, "All Cameras Deleted");
}

bool CameraManager::IsInCamera(Object* object, int index)
{
	if (cameras.empty() == true)
	{
		return false;
	}

	Camera* targetCam = GetCamera(index);

	if (targetCam == nullptr || targetCam->GetIsActive() == false)
	{
		return false;
	}

	glm::vec2 position = { object->GetPosition().x, object->GetPosition().y };
	glm::vec2 size = { object->GetSize().x, object->GetSize().y };
	glm::vec2 viewSize = targetCam->GetViewSize();
	glm::vec2 cameraCenter = targetCam->GetCenter();

	if (position.x - (size.x) < (viewSize.x / 2.f + cameraCenter.x) && position.x + (size.x) > -(viewSize.x / 2.f - cameraCenter.x)
		&& position.y - (size.y) < (viewSize.y / 2.f + cameraCenter.y) && position.y + (size.y) > -(viewSize.y / 2.f - cameraCenter.y))
	{
		return true;
	}
	return false;
}

bool CameraManager::IsScreenPointOccluded(glm::vec2 screenPos, int cameraIndex)
{
	ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
	glm::vec2 windowPos = { imguiViewport->Pos.x, imguiViewport->Pos.y };
	glm::vec2 windowSize = { imguiViewport->Size.x, imguiViewport->Size.y };

	// Check all cameras drawn AFTER the specified camera index
	for (int i = cameraIndex + 1; i < static_cast<int>(cameras.size()); ++i)
	{
		if (cameras[i]->GetIsActive())
		{
			ViewportRect vp = cameras[i]->GetViewport();

			float left = vp.x * windowSize.x + windowPos.x;
			float top = vp.y * windowSize.y + windowPos.y;
			float right = (vp.x + vp.width) * windowSize.x + windowPos.x;
			float bottom = (vp.y + vp.height) * windowSize.y + windowPos.y;

			if (screenPos.x >= left && screenPos.x <= right &&
				screenPos.y >= top && screenPos.y <= bottom)
			{
				return true; // Occluded by a later camera
			}
		}
	}
	return false;
}

void CameraManager::CameraControllerImGui()
{
	if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Q))
	{
		SDL_Window* window = Engine::Instance().GetWindow().GetWindow();

		if (SDL_GetWindowRelativeMouseMode(window) == false)
		{
			Engine::Instance().GetInputManager().SetRelativeMouseMode(true);
		}
		else
		{
			Engine::Instance().GetInputManager().SetRelativeMouseMode(false);
		}
	}

	ImGui::Begin("CameraController");
	if (cameras.empty() == false)
	{
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::BeginCombo("##CameraList", cameras[selectedCameraIndex]->GetName().c_str()))
		{
			for (int i = 0; i < cameras.size(); i++)
			{
				bool isSelected = (selectedCameraIndex == i);
				if (ImGui::Selectable(cameras[i]->GetName().c_str(), isSelected))
				{
					selectedCameraIndex = i;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
	}

	if (ImGui::Button("Add"))
	{
		if(Engine::GetRenderManager()->GetRenderType() == RenderType::ThreeDimension)
		{
			AddCamera(CameraType::ThreeDimension);
			selectedCameraIndex = cameras.size() - 1;
		}
		else
		{
			AddCamera(CameraType::TwoDimension);
			selectedCameraIndex = cameras.size() - 1;
		}
	}

	if (cameras.empty() == false && cameras.size() > 1)
	{
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			DeleteCamera(selectedCameraIndex);
			if (selectedCameraIndex > 0)
			{
				selectedCameraIndex = selectedCameraIndex - 1;
			}
			else if(selectedCameraIndex == 0)
			{
				selectedCameraIndex = 0;
			}
		}
	}

	ImGui::Separator();

	if (cameras.empty() == false)
	{
		Camera* selectedCam = cameras[selectedCameraIndex].get();

		bool isActive = selectedCam->GetIsActive();
		if (ImGui::Checkbox("Is Active", &isActive))
		{
			selectedCam->SetIsActive(isActive);
		}

		if (isActive == true)
		{
			ViewportRect v = selectedCam->GetViewport();
			float vArr[4] = { v.x, v.y, v.width, v.height };

			if (ImGui::DragFloat4("Viewport (X,Y,W,H)", vArr, 0.01f, 0.0f, 1.0f))
			{
				selectedCam->SetViewport(vArr[0], vArr[1], vArr[2], vArr[3]);
			}

			glm::vec3 position = selectedCam->GetCameraPosition();
			float zoom = selectedCam->GetZoom();

			if (selectedCam->GetCameraType() == CameraType::ThreeDimension)
			{
				float nearClip = selectedCam->GetNear();
				float farClip = selectedCam->GetFar();
				float pitch = selectedCam->GetPitch();
				float yaw = selectedCam->GetYaw();

				glm::vec3 cameraOffset = selectedCam->GetCameraOffset();
				float cameraDistance = selectedCam->GetCameraDistance();
				bool isThirdPersonView = selectedCam->GetIsThirdPersonView();

				bool isRelativeOn = Engine::GetInputManager().GetRelativeMouseMode();
				ImGui::Checkbox("Relative Mouse Mod (Press Q)", &isRelativeOn);
				Engine::GetInputManager().SetRelativeMouseMode(isRelativeOn);

				ImGui::Checkbox("Third Person View Mod", &isThirdPersonView);
				selectedCam->SetIsThirdPersonViewMod(isThirdPersonView);

				float cameraSensitivity = selectedCam->GetCameraSensitivity();
				ImGui::SliderFloat("CameraSensitivity", &cameraSensitivity, 0.1f, 100.f);
				selectedCam->SetCameraSensitivity(cameraSensitivity);

				ImGui::DragFloat3("Position", &position.x, 0.01f);
				selectedCam->SetCameraPosition(position);

				ImGui::DragFloat("Zoom", &zoom, 0.5f);
				selectedCam->SetZoom(zoom);

				ImGui::DragFloat("Near", &nearClip, 0.05f);
				selectedCam->SetNear(nearClip);

				ImGui::DragFloat("Far", &farClip, 0.05f);
				selectedCam->SetFar(farClip);

				ImGui::DragFloat("Pitch", &pitch, 0.5f);
				selectedCam->SetPitch(pitch);

				ImGui::DragFloat("Yaw", &yaw, 0.5f);
				selectedCam->SetYaw(yaw);

				if (isThirdPersonView == true && !Engine::GetObjectManager().GetObjectMap().empty())
				{
					if (ImGui::CollapsingHeader("Third Person View Option", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::BeginChild("Scrolling", ImVec2(0, 100));
						int index = 0;

						for (auto& object : Engine::GetObjectManager().GetObjectMap())
						{
							ImGui::PushStyleColor(ImGuiCol_Text, (currentObjIndex == index) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text));

							if (ImGui::Selectable(object.second.get()->GetName().c_str(), index))
							{
								currentObjIndex = index;
							}

							ImGui::PopStyleColor();
							index++;
						}

						ImGui::EndChild();
						selectedCam->SetTarget(Engine::GetObjectManager().FindObjectWithId(currentObjIndex)->GetPosition());

						ImGui::DragFloat("Distance", &cameraDistance, 0.05f);
						selectedCam->SetCameraDistance(cameraDistance);

						ImGui::DragFloat3("Offset", &cameraOffset.x, 0.01f);
						selectedCam->SetCameraOffset(cameraOffset);
					}
				}
			}
			else if (selectedCam->GetCameraType() == CameraType::TwoDimension)
			{
				float rotate2D = selectedCam->GetRotate2D();

				ImGui::DragFloat2("Position", &position.x, 0.1f);
				selectedCam->SetCameraPosition(position);

				ImGui::DragFloat("Zoom", &zoom, 0.1f);
				selectedCam->SetZoom(zoom);

				ImGui::DragFloat("Rotation", &rotate2D, 0.5f);
				selectedCam->Rotate2D(rotate2D);
			}
		}
	}

	DrawCameraDebug();

	ImGui::End();
}

void CameraManager::DrawCameraDebug()
{
	Camera* mainCam = GetCamera(mainCameraIndex);
	if (mainCam == nullptr) return;

	glm::mat4 mainView = mainCam->GetViewMatrix();
	glm::mat4 mainProj = mainCam->GetProjectionMatrix();
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	// Get Main Camera's Viewport for clipping
	ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
	glm::vec2 windowPos = { imguiViewport->Pos.x, imguiViewport->Pos.y };
	glm::vec2 windowSize = { imguiViewport->Size.x, imguiViewport->Size.y };
	ViewportRect mainVp = mainCam->GetViewport();

	ImVec2 clipMin = { mainVp.x * windowSize.x + windowPos.x, mainVp.y * windowSize.y + windowPos.y };
	ImVec2 clipMax = { (mainVp.x + mainVp.width) * windowSize.x + windowPos.x, (mainVp.y + mainVp.height) * windowSize.y + windowPos.y };

	drawList->PushClipRect(clipMin, clipMax);

	for (int i = 0; i < cameras.size(); ++i)
	{
		Camera* cam = cameras[i].get();
		if (cam == nullptr || cam->GetCameraType() != CameraType::ThreeDimension || i == mainCameraIndex) continue;

		// Color: Blue for active, Red for inactive
		ImU32 color = cam->GetIsActive() ? IM_COL32(0, 120, 255, 255) : IM_COL32(255, 0, 0, 255);

		// 1. Draw camera position and name
		glm::vec3 camPos = cam->GetCameraPosition();
		glm::vec2 screenPos = Engine::GetRenderManager()->WorldToScreen(camPos, mainView, mainProj, mainCam);

		if (screenPos.x >= clipMin.x && screenPos.x <= clipMax.x &&
			screenPos.y >= clipMin.y && screenPos.y <= clipMax.y)
		{
			if (!IsScreenPointOccluded(screenPos, mainCameraIndex))
			{
				drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 6.0f, color);
				std::string label = cam->GetName();
				drawList->AddText(ImVec2(screenPos.x + 10.0f, screenPos.y - 10.0f), color, label.c_str());
			}
		}

		// 2. Draw Frustum (Lines for Near, Far, FOV, Aspect)
		glm::mat4 invVP = glm::inverse(cam->GetProjectionMatrix() * cam->GetViewMatrix());

		// Z Range: GL is [-1, 1], others (DX/VK) are [0, 1]
		float nearZ = (Engine::GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL) ? -1.0f : 0.0f;
		glm::vec4 ndc[8] = {
			{-1,-1,nearZ,1}, {1,-1,nearZ,1}, {1,1,nearZ,1}, {-1,1,nearZ,1}, // Near
			{-1,-1,1,1}, {1,-1,1,1}, {1,1,1,1}, {-1,1,1,1}                 // Far
		};

		glm::vec2 screenCorners[8];
		bool hasValidPoint = false;
		for (int j = 0; j < 8; ++j)
		{
			glm::vec4 worldPos = invVP * ndc[j];
			glm::vec3 p = glm::vec3(worldPos) / worldPos.w;
			screenCorners[j] = Engine::GetRenderManager()->WorldToScreen(p, mainView, mainProj, mainCam);
			if (screenCorners[j].x != -1) hasValidPoint = true;
		}

		if (hasValidPoint)
		{
			// Connect Near plane
			for (int j = 0; j < 4; ++j)
				drawList->AddLine(ImVec2(screenCorners[j].x, screenCorners[j].y), ImVec2(screenCorners[(j + 1) % 4].x, screenCorners[(j + 1) % 4].y), color, 1.5f);

			// Connect Far plane
			for (int j = 0; j < 4; ++j)
				drawList->AddLine(ImVec2(screenCorners[j + 4].x, screenCorners[j + 4].y), ImVec2(screenCorners[(j + 1) % 4 + 4].x, screenCorners[(j + 1) % 4 + 4].y), color, 1.5f);

			// Connect Near to Far
			for (int j = 0; j < 4; ++j)
				drawList->AddLine(ImVec2(screenCorners[j].x, screenCorners[j].y), ImVec2(screenCorners[j + 4].x, screenCorners[j + 4].y), color, 1.5f);
		}
	}

	drawList->PopClipRect();
}

void CameraManager::ControlCamera(float dt)
{
	Camera* selectedCam = GetCamera(selectedCameraIndex);

	if (selectedCam == nullptr || selectedCam->GetIsActive() == false)
	{
		return;
	}

	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::W))
	{
		selectedCam->MoveCameraPos(CameraMoveDir::FOWARD, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::S))
	{
		selectedCam->MoveCameraPos(CameraMoveDir::BACKWARD, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::A))
	{
		selectedCam->MoveCameraPos(CameraMoveDir::LEFT, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::D))
	{
		selectedCam->MoveCameraPos(CameraMoveDir::RIGHT, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::SPACE))
	{
		selectedCam->MoveCameraPos(CameraMoveDir::UP, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LSHIFT))
	{
		selectedCam->MoveCameraPos(CameraMoveDir::DOWN, 5.f * dt);
	}
	if (Engine::GetInputManager().GetMouseWheelMotion().y != 0.f)
	{
		selectedCam->SetZoom(selectedCam->GetZoom() + Engine::GetInputManager().GetMouseWheelMotion().y);
	}

	SDL_Window* window = Engine::Instance().GetWindow().GetWindow();

	if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::RIGHT) || SDL_GetWindowRelativeMouseMode(window) == true)
	{
		selectedCam->UpdateCameraDirection(Engine::Instance().GetInputManager().GetRelativeMouseState() * dt);
	}
	//TBD
}