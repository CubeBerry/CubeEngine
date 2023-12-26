#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include "Camera.hpp"

void Camera::Update()
{
	glm::vec3 frontT;
	frontT.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontT.y = sin(glm::radians(pitch));
	frontT.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(frontT) * glm::rotateZ(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(rotate2D));
	std::cout << rotate2D << std::endl;
	switch (cameraType)
	{
	case CameraType::TwoDimension:
		view = glm::lookAt(cameraPosition, cameraPosition + front, upVector);
		projection = glm::perspective(glm::radians(zoom), aspectRatio, nearClip, farClip);
		break;
	case CameraType::ThreeDimension:
		view = glm::lookAt(cameraPosition, cameraTarget + front, upVector);
		projection = glm::perspective(glm::radians(zoom), aspectRatio, nearClip, farClip);
		break;
	default:
		break;
	}
}

void Camera::SetCenter(glm::vec3 centerPosition) noexcept
{
	this->cameraTarget = centerPosition;
	this->cameraPosition = centerPosition;
}

void Camera::MoveUp(float distance) noexcept
{
	switch (cameraType)
	{
	case CameraType::TwoDimension:
		cameraPosition += normalize(upVector) * distance;
		break;
	case CameraType::ThreeDimension:
		cameraPosition += front * distance;
		break;
	default:
		break;
	}
}

void Camera::MoveRight(float distance) noexcept
{
	switch (cameraType)
	{
	case CameraType::TwoDimension:
		cameraPosition += normalize(rightVector) * distance;
		break;
	case CameraType::ThreeDimension:
		cameraPosition += normalize(glm::cross(front, upVector)) * distance;
		break;
	default:
		break;
	}
}

void Camera::Rotate(float angle) noexcept
{
	switch (cameraType)
	{
	case CameraType::TwoDimension:
		rotate2D = angle;
		break;
	case CameraType::ThreeDimension:
		break;
	default:
		break;
	}
}

void Camera::ResetUp(glm::vec3 startUpPosition)
{
	this->upVector.x = startUpPosition.x;
	this->upVector.y = startUpPosition.y;
	this->upVector.z = startUpPosition.z;
	rightVector = { startUpPosition.y, -startUpPosition.x, startUpPosition.z };
	//need to fix it in 3d
}

void Camera::SetViewSize(int width, int height) noexcept
{
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	//aspectRatio = aspectRatio > 1.0f ? aspectRatio : 1.0f / aspectRatio;
}

void Camera::SetZoom(float amount) noexcept
{
	zoom = glm::clamp(amount, nearClip, farClip);
}