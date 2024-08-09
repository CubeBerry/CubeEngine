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

#ifdef _DEBUG
void CameraManager::CameraControllerImGui()
{
	float targetP[3] = {GetCameraPosition().x, GetCameraPosition().y, GetCameraPosition().z};
	float zoom = GetZoom();
	float nearClip = GetNear();
	float farClip = GetFar();
	float pitch = GetPitch();
	float yaw = GetYaw();
	//float cameraSensitivity = GetCameraSensitivity();

	ImGui::Begin("CameraController");

	ImGui::InputFloat3("Position", targetP);
	SetCameraPosition(glm::vec3{ targetP[0], targetP[1], targetP[2] });

	ImGui::InputFloat("Zoom", &zoom, 0.5f, 1.0f);
	SetZoom(zoom);

	ImGui::InputFloat("NearClip", &nearClip, 1.0f, 1.0f);
	SetNear(nearClip);
	ImGui::InputFloat("FarClip", &farClip, 1.0f, 1.0f);
	SetFar(farClip);

	ImGui::InputFloat("Pitch", &pitch, 1.0f, 1.0f);
	SetPitch(pitch);
	ImGui::InputFloat("Yaw", &yaw, 1.0f, 1.0f);
	SetYaw(yaw);

	ImGui::End();
}
#endif

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
	//if (Engine::GetInputManager().IsKeyPressOnce(KEYBOARDKEYS::Q))
	//{
	//	if (SDL_GetRelativeMouseMode() == SDL_FALSE)
	//	{
	//		Engine::Instance().GetInputManager().SetRelativeMouseMode(true);
	//	}
	//	else
	//	{
	//		Engine::Instance().GetInputManager().SetRelativeMouseMode(false);
	//	}
	//}
	if (Engine::GetInputManager().GetMouseWheelMotion().y != 0.f)
	{
		SetZoom(GetZoom() + Engine::GetInputManager().GetMouseWheelMotion().y);
	}
	if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::RIGHT))
	{
		UpdaetCameraDirectrion(Engine::Instance().GetInputManager().GetRelativeMouseState() * dt);
	}
}
