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
	void Update(); //2D, 3D
	void TargetAt(glm::vec3 targetLocation); //2D, 3D
	void Reset(); //2D, 3D

	void MoveCameraPos(CameraMoveDir dir, float speed) { camera.MoveCameraPos(dir, speed); } //3D
	void UpdaetCameraDirectrion(glm::vec2 dir) { camera.UpdaetCameraDirectrion(dir); } //3D

	void SetRotate2D(float angle) noexcept { camera.Rotate2D(angle); } //2D
	void SetZoom(float zoom) noexcept { camera.SetZoom(zoom); } //2D, 3D
	void SetViewSize(int width, int height) noexcept { camera.SetViewSize(width, height); } //2D, 3D
	void SetCenter(glm::vec3 pos) { camera.SetCenter(pos); } //2D, 3D
	void SetCameraPosition(glm::vec3 pos) { camera.SetCameraPosition(pos); } //2D, 3D
	void SetCameraSensitivity(float amount) noexcept { camera.SetCameraSensitivity(amount); } //3D

	float GetRotate2D() { return camera.GetRotate2D(); } //2D
	float GetZoom() noexcept { return camera.GetZoom(); } //2D, 3D
	glm::vec3 GetCenter() { return camera.GetCenter(); } //2D, 3D
	glm::vec3 GetCameraPosition() { return camera.GetCameraPosition(); } //2D, 3D
	glm::vec2 GetViewSize() { return camera.GetViewSize(); } //2D, 3D
	float GetCameraSensitivity() noexcept { return camera.GetCameraSensitivity(); } //3D

	glm::mat4		GetViewMatrix() { return camera.GetViewMatrix(); } //2D, 3D
	glm::mat4		GetProjectionMatrix() { return camera.GetProjectionMatrix(); } //2D, 3D

	void            SetCameraCenterMode(CameraCenterMode cameraCenterMode) noexcept { camera.SetCameraCenterMode(cameraCenterMode); } //2D
	bool IsInCamera(Object* object); //2D (TBD in 3D)
private:
	Camera camera;
};
