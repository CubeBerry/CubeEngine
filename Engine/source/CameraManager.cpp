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

void CameraManager::TargetAt(glm::vec3 targetLocation)
{
	camera.SetCenter(targetLocation);
}

void CameraManager::Reset()
{
	camera.Reset({0.f,1.f,0.1f}); 
	SetCameraSensitivity(1.f);
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
	if (Engine::GetInputManager().IsMouseButtonPressed(MOUSEBUTTON::LEFT))
	{
		UpdaetCameraDirectrion(Engine::Instance().GetInputManager().GetRelativeMouseState() * dt);
	}
}
