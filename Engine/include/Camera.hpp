#pragma once
#include "glm/glm.hpp"

enum class CameraType
{
	TwoDimension,
	ThreeDimension
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
	~Camera();
	//CAMERA
	glm::vec3 GetCenter() const noexcept { return cameraTarget; }
	void    SetCenter(glm::vec3 centerPosition) noexcept { this->cameraTarget = centerPosition; }

	//glm::vec3 GetUp() const noexcept { return up; }
	//glm::vec3 GetRight() const noexcept { return right; }  
	//void ResetUp(glm::vec3 startUpPosition);

	void MoveUp(float distance) noexcept;
	void MoveRight(float distance) noexcept;
	void Rotate(float angle) noexcept;

	glm::mat3 CameraToWorld() const noexcept;
	glm::mat3 WorldToCamera() const noexcept;
	//CAMERA

	//VIEW
	void            SetViewSize(int width, int height) noexcept;
	void            SetZoom(float amount) noexcept;
	//constexpr float GetZoom() const noexcept { return zoom; }

	//glm::mat3 GetCameraToNDC() const noexcept { return NDC; }
	glm::mat3 BuildToNDC(glm::vec3 size);

	void                       SetCameraCenterMode(CameraCenterMode cameraMode) noexcept;
	constexpr CameraCenterMode GetCameraCenterMode() const noexcept { return cameraCenterMode; }
	//VIEW
private:
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 0.0f };
	glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f };
	glm::vec3 upVector{ 0.0f, 1.0f, 0.0f };

	float fov = 45.0f;
	float aspectRatio = 0.f; // Replace screenWidth and screenHeight with actual values
	float nearClip = 0.1f;
	float farClip = 100.0f;

	CameraCenterMode cameraCenterMode = RightOriginCenter;
};