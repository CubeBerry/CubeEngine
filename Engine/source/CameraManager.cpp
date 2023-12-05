#include "CameraManager.hpp"

void CameraManager::Init(glm::vec2 viewSize, CameraType type, float zoom, float angle)
{
	camera.SetCameraType(type);
	camera.SetViewSize(viewSize.x, viewSize.y);
	camera.SetZoom(zoom);
	camera.Rotate(angle);
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
	camera.ResetUp({0.f,1.f,0.1f});
	camera.SetCenter({0.f,0.f,0.f});
	camera.SetZoom(1.f);

}
