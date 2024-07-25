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

	void Update();

	glm::vec3 GetCenter() const noexcept { return cameraTarget; }
	void    SetCenter(glm::vec3 centerPosition) noexcept;

	glm::vec3 GetCameraPosition() const noexcept { return cameraPosition; }
	void    SetCameraPosition(glm::vec3 cameraPosition_) noexcept;

	glm::vec3 GetUp() const noexcept { return up; }
	glm::vec3 GetRight() const noexcept { return right; }

	void MoveCameraPos(CameraMoveDir dir, float speed);
	void UpdaetCameraDirectrion(glm::vec2 dir);

	void Rotate(float angle) noexcept;
	void Reset(glm::vec3 startUpPosition);

	glm::mat4		GetViewMatrix() { return view; }
	glm::mat4		GetProjectionMatrix() { return projection; }
	void            SetViewSize(int width, int height) noexcept;
	glm::vec2		GetViewSize() { return cameraViewSize; }
	float			GetZoom() { return zoom; }
	float			GetCameraSensitivity() { return cameraSensitivity; }
	void            SetCameraCenterMode(CameraCenterMode mode) noexcept { cameraCenterMode = mode; };
	constexpr CameraCenterMode GetCameraCenterMode() const noexcept { return cameraCenterMode; }

	float GetRotate2D() { return rotate2D; }
	void			SetCameraType(CameraType type) { cameraType = type; }
	void            SetNear(float amount) noexcept { nearClip = amount; }
	void            SetFar(float amount) noexcept { farClip = amount; }
	void            SetZoom(float amount) noexcept;
	void            SetCameraSensitivity(float amount) noexcept { cameraSensitivity = amount; }
private:
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 1.0f };
	glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f };

	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 front{ 0.0f, 0.0f, -1.0f };
	glm::vec3 right{ 1.0f, 0.0f, 0.0f };
	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

	float zoom = 1.0f;
	float aspectRatio = 1.f;
	float nearClip = 0.1f;
	float farClip = 45.0f;
	float pitch = 0.0f;
	float yaw = -90.0f;
	float rotate2D = 0.f;
	float cameraSensitivity = 1.f;

	glm::mat4 view = glm::mat4(0.f);
	glm::mat4 projection = glm::mat4(0.f);
	glm::vec2 cameraViewSize = glm::vec2(0.f);
	CameraType cameraType = CameraType::NONE;
	CameraCenterMode cameraCenterMode = RightOriginCenter;
};