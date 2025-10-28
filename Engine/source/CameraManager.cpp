//Author: DOYEONG LEE
//Project: CubeEngine
//File: CameraManager.cpp
#include "CameraManager.hpp"
#include "Engine.hpp"

void CameraManager::Init(glm::vec2 viewSize, CameraType type, float zoom, float angle)
{
	camera.SetCameraType(type);
	camera.SetViewSize(static_cast<int>(viewSize.x), static_cast<int>(viewSize.y));
	camera.SetZoom(zoom);
	camera.Rotate2D(angle);

	switch (type)
	{
	case CameraType::TwoDimension:
		break;
	case CameraType::ThreeDimension:
		break;
	default:
		break;
	}
	Engine::GetDebugLogger().LogDebug(LogCategory::Engine, "Camera Manager Initialized!");
}

void CameraManager::Update()
{
	camera.Update();
}

void CameraManager::Reset()
{
	camera.Reset(); 
}

bool CameraManager::IsInCamera(Object* object)
{
	glm::vec2 position = { object->GetPosition().x, object->GetPosition().y };
	glm::vec2 size = { object->GetSize().x, object->GetSize().y };
	glm::vec2 viewSize = GetViewSize();
	glm::vec2 cameraCenter = GetCenter();

	if (position.x - (size.x) < (viewSize.x / 2.f + cameraCenter.x) && position.x + (size.x) > -(viewSize.x / 2.f - cameraCenter.x)
		&& position.y - (size.y) < (viewSize.y / 2.f + cameraCenter.y) && position.y + (size.y) > -(viewSize.y / 2.f - cameraCenter.y))
	{
		return true;
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

	glm::vec3 position = GetCameraPosition();
	float zoom = GetZoom();
	float nearClip = GetNear();
	float farClip = GetFar();
	float pitch = GetPitch();
	float yaw = GetYaw();
	float cameraSensitivity = GetCameraSensitivity();
	bool isRelativeOn = Engine::GetInputManager().GetRelativeMouseMode();

	glm::vec3 cameraOffset = GetCameraOffset(); 
	float cameraDistance = GetCameraDistance();
	bool isThirdPersonView = GetIsThirdPersonView();

	ImGui::Begin("CameraController");

	ImGui::Checkbox("Third Person View Mod", &isThirdPersonView);
	SetIsThirdPersonViewMod(isThirdPersonView);

	ImGui::Checkbox("Relative Mouse Mod (Press Q)", &isRelativeOn);
	Engine::GetInputManager().SetRelativeMouseMode(isRelativeOn);

	ImGui::SliderFloat("CameraSensitivity", &cameraSensitivity, 0.1f, 100.f);
	SetCameraSensitivity(cameraSensitivity);

	ImGui::DragFloat3("Position", &position.x, 0.01f);
	SetCameraPosition(position);

	ImGui::DragFloat("Zoom", &zoom, 0.5f);
	SetZoom(zoom);

	ImGui::DragFloat("Near", &nearClip, 0.05f);
	SetNear(nearClip);

	ImGui::DragFloat("Far", &farClip, 0.05f);
	SetFar(farClip);

	ImGui::DragFloat("Pitch", &pitch, 0.5f);
	SetPitch(pitch);

	ImGui::DragFloat("Yaw", &yaw, 0.5f);
	SetYaw(yaw);
	
	if (isThirdPersonView == true && !Engine::GetObjectManager().GetObjectMap().empty())
	{
		if (ImGui::CollapsingHeader("Third Person View Option", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("Scolling");
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
			SetTarget(Engine::GetObjectManager().FindObjectWithId(currentObjIndex)->GetPosition());

			ImGui::DragFloat("Distance", &cameraDistance, 0.05f);
			SetCameraDistance(cameraDistance);
			ImGui::DragFloat3("Offset", &cameraOffset.x, 0.01f);
			SetCameraOffset(cameraOffset);
		}
	}
	ImGui::End();
}

void CameraManager::ControlCamera(float dt)
{
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::W))
	{
		MoveCameraPos(CameraMoveDir::FOWARD, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::S))
	{
		MoveCameraPos(CameraMoveDir::BACKWARD, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::A))
	{
		MoveCameraPos(CameraMoveDir::LEFT, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::D))
	{
		MoveCameraPos(CameraMoveDir::RIGHT, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::SPACE))
	{
		MoveCameraPos(CameraMoveDir::UP, 5.f * dt);
	}
	if (Engine::GetInputManager().IsKeyPressed(KEYBOARDKEYS::LSHIFT))
	{
		MoveCameraPos(CameraMoveDir::DOWN, 5.f * dt);
	}
	if (Engine::GetInputManager().GetMouseWheelMotion().y != 0.f)
	{
		SetZoom(GetZoom() + Engine::GetInputManager().GetMouseWheelMotion().y);
	}
	SDL_Window* window = Engine::Instance().GetWindow().GetWindow();
	if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::RIGHT) || SDL_GetWindowRelativeMouseMode(window) == true)
	{
		UpdaetCameraDirectrion(Engine::Instance().GetInputManager().GetRelativeMouseState() * dt);
	}
	//TBD
}
