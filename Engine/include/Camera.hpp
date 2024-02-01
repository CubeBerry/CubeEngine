//Author: DOYEONG LEE
//Project: CubeEngine
//File: Camera.hpp
#pragma once
#include "glm/glm.hpp"

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

	void Update();

	glm::vec3 GetCenter() const noexcept { return cameraTarget; }
	void    SetCenter(glm::vec3 centerPosition) noexcept;

	glm::vec3 GetUp() const noexcept { return upVector; }
	glm::vec3 GetRight() const noexcept { return rightVector; }
	
	void MoveUp(float distance) noexcept;
	void MoveRight(float distance) noexcept;
	void Rotate(float angle) noexcept;
	void ResetUp(glm::vec3 startUpPosition);

	glm::mat4		GetViewMatrix() { return view; }
	glm::mat4		GetProjectionMatrix() { return projection; }
	void            SetViewSize(int width, int height) noexcept;
	glm::vec2		GetViewSize() { return cameraViewSize; }
	void            SetZoom(float amount) noexcept;
	float			GetZoom() { return zoom; }
	void                       SetCameraCenterMode(CameraCenterMode cameraMode) noexcept {};
	constexpr CameraCenterMode GetCameraCenterMode() const noexcept { return cameraCenterMode; }

	float GetRotate2D() { return rotate2D; }
	void			SetCameraType(CameraType type) { cameraType = type; }
	void            SetNear(float amount) noexcept { nearClip = amount; }
	void            SetFar(float amount) noexcept { farClip = amount; }
	void            SetZoom(glm::vec2 size) noexcept { cameraViewSize = size; }
private:
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 1.0f };
	glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f };
	glm::vec3 front{ 0.0f, 0.0f, -1.0f };
	glm::vec3 upVector{ 0.0f, 1.0f, 0.0f };
	glm::vec3 rightVector{ 1.0f, 0.0f, 0.0f };

	float zoom = 1.0f;
	float aspectRatio = 1.f;
	float nearClip = 0.1f;
	float farClip = 100.0f;
	float pitch = 0.0f;
	float yaw = -90.0f;
	float rotate2D = 0.f;

	glm::mat4 view;
	glm::mat4 projection;
	glm::vec2 cameraViewSize;
	CameraType cameraType = CameraType::NONE;
	CameraCenterMode cameraCenterMode = RightOriginCenter;
};