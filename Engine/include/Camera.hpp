//Author: DOYEONG LEE
//Project: CubeEngine
//File: Camera.hpp
#pragma once
#include "glm/glm.hpp"

enum class CameraMoveDir
{
	FOWARD,
	BACKWARD,
	UP,
	DOWN,
	LEFT,
	RIGHT
};

enum class CameraType
{
	TwoDimension,
	ThreeDimension,
	NONE
};

enum CameraCenterMode
{
	RightOriginCenter,
	RightOriginBottomLeft,
	LeftOriginTopLeft,
	NormalizedDeviceCoordinates
};

class Camera
{
public:
	constexpr Camera() noexcept = default;
	~Camera() {};

	void Update(); //2D, 3D

	glm::vec3 GetCenter() const noexcept { return cameraTarget; } //2D, 3D
	void    SetCenter(glm::vec3 centerPosition) noexcept; //2D, 3D

	glm::vec3 GetCameraPosition() const noexcept { return cameraPosition; } //2D, 3D
	void    SetCameraPosition(glm::vec3 cameraPosition_) noexcept; //2D, 3D

	glm::vec3 GetUp() const noexcept { return up; } //2D, 3D
	glm::vec3 GetRight() const noexcept { return right; } //2D, 3D

	void MoveCameraPos(CameraMoveDir dir, float speed); //2D, 3D
	void UpdaetCameraDirectrion(glm::vec2 dir); //3D 

	void Rotate2D(float angle) noexcept; //2D
	void Reset(glm::vec3 startUpPosition); //2D, 3D

	glm::mat4		GetViewMatrix() { return view; }
	glm::mat4		GetProjectionMatrix() { return projection; }
	void            SetViewSize(int width, int height) noexcept;
	glm::vec2		GetViewSize() { return cameraViewSize; }
	float			GetZoom() { return zoom; }
	float			GetCameraSensitivity() { return cameraSensitivity; } //3D
	void            SetCameraCenterMode(CameraCenterMode mode) noexcept { cameraCenterMode = mode; }; //2D (TBD)
	constexpr CameraCenterMode GetCameraCenterMode() const noexcept { return cameraCenterMode; } //2D (TBD)

	float GetRotate2D() { return rotate2D; } //2D 
	void			SetCameraType(CameraType type) { cameraType = type; }
	void            SetNear(float amount) noexcept { nearClip = amount; } //3D
	void            SetFar(float amount) noexcept { farClip = amount; } //3D
	void            SetZoom(float amount) noexcept; //2D, 3D
	void            SetCameraSensitivity(float amount) noexcept { cameraSensitivity = amount; } //3D
private:
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 1.0f }; //2D, 3D
	glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f }; //3D

	glm::vec3 up{ 0.0f, 1.0f, 0.0f }; //2D, 3D
	glm::vec3 back{ 0.0f, 0.0f, -1.0f }; //3D
	glm::vec3 right{ 1.0f, 0.0f, 0.0f }; //2D, 3D
	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f }; //3D

	float zoom = 1.0f; //2D, 3D
	float aspectRatio = 1.f; //3D (TBD)
	float nearClip = 0.1f; //3D
	float farClip = 45.0f; //3D
	float pitch = 0.0f; //3D
	float yaw = -90.0f; //3D
	float rotate2D = 0.f; //2D
	float cameraSensitivity = 1.f; //3D

	glm::mat4 view = glm::mat4(0.f); //2D, 3D
	glm::mat4 projection = glm::mat4(0.f); //2D, 3D
	glm::vec2 cameraViewSize = glm::vec2(0.f); //2D, 3D
	CameraType cameraType = CameraType::NONE; //2D, 3D
	CameraCenterMode cameraCenterMode = RightOriginCenter; //2D
};