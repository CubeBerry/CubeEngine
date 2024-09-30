//Author: DOYEONG LEE
//Project: CubeEngine
//File: Camera.hpp
#pragma once
#include "3DPhysicsDemo.hpp"
#include "3DPhysicsDemo.hpp"
#include "glm/glm.hpp"
#include "Ray.hpp"

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
	void Reset();

	//2D, 3D
	void    SetTarget(glm::vec3 pos);
	void    SetCameraPosition(glm::vec3 cameraPosition_) noexcept; 
	void    SetViewSize(int width, int height) noexcept;
	void    SetZoom(float amount) noexcept; 

	glm::vec3 GetCenter() const noexcept { return cameraCenter; }
	glm::vec3 GetCameraPosition() const noexcept { return cameraPosition; } 
	glm::vec3 GetUp() const noexcept { return up; } 
	glm::vec3 GetRight() const noexcept { return right; } 

	glm::mat4 GetViewMatrix() { return view; }
	glm::mat4 GetProjectionMatrix() { return projection; }
	glm::vec2 GetViewSize() { return cameraViewSize; }
	float	  GetZoom() { return zoom; }

	void MoveCameraPos(CameraMoveDir dir, float speed); 
 
	//2D
	void Rotate2D(float angle) noexcept;
	float GetRotate2D() { return rotate2D; } 

	void  SetCameraCenterMode(CameraCenterMode mode) noexcept { cameraCenterMode = mode; }; //(TBD)
	constexpr CameraCenterMode GetCameraCenterMode() const noexcept { return cameraCenterMode; } //(TBD)
	
	//3D
	void LookAt(glm::vec3 pos);
	float			GetCameraSensitivity() { return cameraSensitivity; }

	void			SetCameraType(CameraType type) { cameraType = type; }
	void            SetNear(float amount) noexcept { nearClip = amount; }
	void            SetFar(float amount) noexcept { farClip = amount; }
	void            SetPitch(float amount) noexcept { pitch = amount; }
	void            SetYaw(float amount) noexcept { yaw = amount; }
	void            SetBaseFov(float amount) noexcept { baseFov = amount; }
	void            SetCameraSensitivity(float amount) noexcept { cameraSensitivity = amount; }
	void			UpdaetCameraDirectrion(glm::vec2 dir);
	void			SetIsThirdPersonViewMod(bool state) { isThirdPersonView = state; }
	void            SetCameraDistance(float amount) noexcept { cameraDistance = amount; }
	void            SetCameraOffset(glm::vec3 amount) noexcept { cameraOffset = amount; }

	float	  GetNear() { return nearClip; }
	float	  GetFar() { return farClip; }
	float	  GetPitch() { return pitch; }
	float	  GetYaw() { return yaw; }
	float	  GetBaseFov() { return baseFov; }
	float	  GetIsThirdPersonView() { return isThirdPersonView; }
	float	  GetCameraDistance() { return cameraDistance; }
	glm::vec3 GetCameraOffset() { return cameraOffset; }

	glm::vec3 GetUpVector() const { return up; }
	glm::vec3 GetBackVector() const { return back; }
	glm::vec3 GetRightVector() const { return right; }

	Ray CalculateRayFrom2DPosition(glm::vec2 pos);
private:
	//2D, 3D
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 0.0f }; 
	glm::vec3 up{ 0.0f, 1.0f, 0.0f }; 
	glm::vec3 right{ 1.0f, 0.0f, 0.0f }; 
	float zoom = 1.0f;
	float baseFov = 45.f;

	glm::mat4 view = glm::mat4(0.f); 
	glm::mat4 projection = glm::mat4(0.f); 
	glm::vec2 cameraViewSize = glm::vec2(0.f); 
	CameraType cameraType = CameraType::NONE; 
	
	//2D
	float rotate2D = 0.f; 
	CameraCenterMode cameraCenterMode = RightOriginCenter; 

	//3D
	glm::vec3 cameraCenter{ 0.0f, 0.0f, 0.0f };
	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
	glm::vec3 back{ 0.0f, 0.0f, -1.0f };
	glm::vec3 cameraOffset{ 0.0f, 0.0f, 0.0f }; //In ThirdPersonView

	float aspectRatio = 1.f; //(TBD)
	float nearClip = 1.f;
	float farClip = 45.0f;
	float pitch = 0.0f;
	float yaw = -90.0f;
	float cameraSensitivity = 1.f;
	float cameraDistance = 5.0f; //In ThirdPersonView
	bool isThirdPersonView = false; //In ThirdPersonView
};