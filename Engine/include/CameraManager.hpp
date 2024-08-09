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
	void SetZoom(float zoom) noexcept { camera.SetZoom(zoom); }
	void SetViewSize(int width, int height) noexcept { camera.SetViewSize(width, height); }
	void SetCenter(glm::vec3 pos, bool isCenterFollow = false) { camera.SetCenter(pos, isCenterFollow); }
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

	void SetNear(float amount) noexcept { camera.SetNear(amount); }
	void SetFar(float amount) noexcept { camera.SetFar(amount); }
	void SetPitch(float amount) noexcept { camera.SetPitch(amount); }
	void SetYaw(float amount) noexcept { camera.SetYaw(amount); }
	void SetBaseFov(float amount) noexcept { camera.SetBaseFov(amount); }

	float GetNear() noexcept { return camera.GetNear(); }
	float GetFar() noexcept { return camera.GetFar(); }
	float GetPitch() noexcept { return camera.GetPitch(); }
	float GetYaw() noexcept { return camera.GetYaw(); }
	float GetBaseFov() { return camera.GetBaseFov(); }

#ifdef _DEBUG
	void CameraControllerImGui();
#endif
private:
	Camera camera;
};
