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
	void Reset();

	// 2D, 3D
	void TargetAt(glm::vec3 targetLocation); 

	void SetZoom(float zoom) noexcept { camera.SetZoom(zoom); } 
	void SetViewSize(int width, int height) noexcept { camera.SetViewSize(width, height); } 
	void SetCenter(glm::vec3 pos) { camera.SetCenter(pos); } 
	void SetCameraPosition(glm::vec3 pos) { camera.SetCameraPosition(pos); } 

	float GetZoom() noexcept { return camera.GetZoom(); } 
	glm::vec2 GetViewSize() { return camera.GetViewSize(); } 
	glm::vec3 GetCenter() { return camera.GetCenter(); } 
	glm::vec3 GetCameraPosition() { return camera.GetCameraPosition(); } 
	glm::mat4 GetViewMatrix() { return camera.GetViewMatrix(); } 
	glm::mat4 GetProjectionMatrix() { return camera.GetProjectionMatrix(); } 

	void ControlCamera(float dt);
	
	//2D
	float GetRotate2D() { return camera.GetRotate2D(); } 
	void SetRotate2D(float angle) noexcept { camera.Rotate2D(angle); }

	void SetCameraCenterMode(CameraCenterMode cameraCenterMode) noexcept { camera.SetCameraCenterMode(cameraCenterMode); }
	bool IsInCamera(Object* object); //2D (TBD in 3D)

	//3D
	void MoveCameraPos(CameraMoveDir dir, float speed) { camera.MoveCameraPos(dir, speed); }
	void UpdaetCameraDirectrion(glm::vec2 dir) { camera.UpdaetCameraDirectrion(dir); } 
	void SetCameraSensitivity(float amount) noexcept { camera.SetCameraSensitivity(amount); } 
	float GetCameraSensitivity() noexcept { return camera.GetCameraSensitivity(); } 
private:
	Camera camera;
};
