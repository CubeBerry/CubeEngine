//Author: DOYEONG LEE
//Project: CubeEngine
//File: CameraManager.hpp
#pragma once
#include "Camera.hpp"

class Object;
class CameraManager
{
public:
	void Init(glm::vec2 viewSize, CameraType type = CameraType::TwoDimension, float zoom = 45.f, float angle = 0.f);
	void Update();
	void TargetAt(glm::vec3 targetLocation);
	void Reset();

	void MoveCameraPos(CameraMoveDir dir, float speed) { camera.MoveCameraPos(dir, speed); }
	void UpdaetCameraDirectrion(glm::vec2 dir) { camera.UpdaetCameraDirectrion(dir); }

	void SetRotate(float angle) noexcept { camera.Rotate(angle); }
	void SetZoom(float zoom) noexcept { camera.SetZoom(zoom); }
	void SetViewSize(int width, int height) noexcept { camera.SetViewSize(width, height); }
	void SetCenter(glm::vec3 pos) { camera.SetCenter(pos); }
	void SetCameraPosition(glm::vec3 pos) { camera.SetCameraPosition(pos); }
	void SetCameraSensitivity(float amount) noexcept { camera.SetCameraSensitivity(amount); }

	float GetRotate2D() { return camera.GetRotate2D(); }
	float GetZoom() noexcept { return camera.GetZoom(); }
	glm::vec3 GetCenter() { return camera.GetCenter(); }
	glm::vec3 GetCameraPosition() { return camera.GetCameraPosition(); }
	glm::vec2 GetViewSize() { return camera.GetViewSize(); }
	float GetCameraSensitivity() noexcept { return camera.GetCameraSensitivity(); }

	glm::mat4		GetViewMatrix() { return camera.GetViewMatrix(); }
	glm::mat4		GetProjectionMatrix() { return camera.GetProjectionMatrix(); }

	void            SetCameraCenterMode(CameraCenterMode cameraCenterMode) noexcept { camera.SetCameraCenterMode(cameraCenterMode); }
	bool IsInCamera(Object* object);
private:
	Camera camera;
};
