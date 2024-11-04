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
	void SetTarget(glm::vec3 pos) { camera.SetTarget(pos); }
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
	void LookAt(glm::vec3 pos) { camera.LookAt(pos); }

	void MoveCameraPos(CameraMoveDir dir, float speed) { camera.MoveCameraPos(dir, speed); }
	void UpdaetCameraDirectrion(glm::vec2 dir) { camera.UpdaetCameraDirectrion(dir); } 
	void SetCameraSensitivity(float amount) noexcept { camera.SetCameraSensitivity(amount); } 
	float GetCameraSensitivity() noexcept { return camera.GetCameraSensitivity(); } 

	void SetNear(float amount) noexcept { camera.SetNear(amount); }
	void SetFar(float amount) noexcept { camera.SetFar(amount); }
	void SetPitch(float amount) noexcept { camera.SetPitch(amount); }
	void SetYaw(float amount) noexcept { camera.SetYaw(amount); }
	void SetBaseFov(float amount) noexcept { camera.SetBaseFov(amount); }
	void SetIsThirdPersonViewMod(bool state) { camera.SetIsThirdPersonViewMod(state); }
	void  SetCameraDistance(float amount) noexcept { camera.SetCameraDistance(amount); }
	void  SetCameraOffset(glm::vec3 amount) noexcept { camera.SetCameraOffset(amount); }

	float GetNear() noexcept { return camera.GetNear(); }
	float GetFar() noexcept { return camera.GetFar(); }
	float GetPitch() noexcept { return camera.GetPitch(); }
	float GetYaw() noexcept { return camera.GetYaw(); }
	float GetBaseFov() { return camera.GetBaseFov(); }
	float GetIsThirdPersonView() { return camera.GetIsThirdPersonView();}
	float GetCameraDistance() { return camera.GetCameraDistance(); }
	glm::vec3 GetCameraOffset() { return camera.GetCameraOffset(); }

	glm::vec3 GetUpVector() const { return camera.GetUpVector(); }
	glm::vec3 GetBackVector() const { return camera.GetBackVector(); }
	glm::vec3 GetRightVector() const { return camera.GetRightVector(); }

	Ray CalculateRayFrom2DPosition(glm::vec2 pos) { return camera.CalculateRayFrom2DPosition(pos); }
	void CameraControllerImGui();
	int currentObjIndex = 0;
private:
	Camera camera;
};
