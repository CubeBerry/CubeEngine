/**************************************************************************************
 *	File Name        : Camera.hpp
 *	Project Name     : Keyboard Warriors
 *	Primary Author   : JeongHak Kim
 *	Secondary Author : 
 *	Copyright Information :
 *    "All content 2019 DigiPen (USA) Corporation, all rights reserved."
 **************************************************************************************/

#pragma once
#include "Camera.hpp"

class CameraManager
{
public:
	void Init(glm::vec2 viewSize, float zoom = 45.f, float angle = 0.f);
	void Update();
	void TargetAt(glm::vec3 targetLocation);
	void Reset();

	void MoveUp(float amount) { camera.MoveUp(amount); }
	void MoveRight(float amount) { camera.MoveRight(amount); }

	void SetRotate(float angle) noexcept { camera.Rotate(angle); }
	void SetZoom(float zoom) noexcept { camera.SetZoom(zoom); }
	void SetViewSize(int width, int height) noexcept { camera.SetViewSize(width, height); }

	float GetZoom() noexcept { return camera.GetZoom(); }
	glm::vec3 GetCenter() { return camera.GetCenter(); }

	glm::mat4		GetViewMatrix() { return camera.GetViewMatrix(); }
	glm::mat4		GetProjectionMatrix() { return camera.GetProjectionMatrix(); }

	void                       SetCameraCenterMode(CameraCenterMode cameraCenterMode) noexcept { camera.SetCameraCenterMode(cameraCenterMode); }
private:
	Camera camera;
};
