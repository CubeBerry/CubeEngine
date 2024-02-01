//Author: DOYEONG LEE
//Project: CubeEngine
//File: CameraManager.hpp
#pragma once
#include "Camera.hpp"

class CameraManager
{
public:
	void Init(glm::vec2 viewSize, CameraType type = CameraType::TwoDimension, float zoom = 45.f, float angle = 0.f);
	void Update();
	void TargetAt(glm::vec3 targetLocation);
	void Reset();

	void MoveUp(float amount) { camera.MoveUp(amount); }
	void MoveRight(float amount) { camera.MoveRight(amount); }

	void SetRotate(float angle) noexcept { camera.Rotate(angle); }
	void SetZoom(float zoom) noexcept { camera.SetZoom(zoom); }
	void SetViewSize(int width, int height) noexcept { camera.SetViewSize(width, height); }

	float GetRotate2D() { return camera.GetRotate2D(); }
	float GetZoom() noexcept { return camera.GetZoom(); }
	glm::vec3 GetCenter() { return camera.GetCenter(); }
	glm::vec2 GetViewSize() { return camera.GetViewSize(); }

	glm::mat4		GetViewMatrix() { return camera.GetViewMatrix(); }
	glm::mat4		GetProjectionMatrix() { return camera.GetProjectionMatrix(); }

	void                       SetCameraCenterMode(CameraCenterMode cameraCenterMode) noexcept { camera.SetCameraCenterMode(cameraCenterMode); }
private:
	Camera camera;
};
