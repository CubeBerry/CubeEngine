#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "Camera.hpp"

void Camera::Update()
{
	glm::vec3 frontT;
	frontT.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontT.y = sin(glm::radians(pitch));
	frontT.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(frontT);

	view = glm::lookAt(cameraPosition, cameraTarget + front, upVector);
	projection = glm::perspective(glm::radians(zoom), aspectRatio, nearClip, farClip);
}

void Camera::MoveUp(float distance) noexcept
{
	cameraPosition += normalize(upVector) * distance;
}

void Camera::MoveRight(float distance) noexcept
{
	cameraPosition += normalize(rightVector) * distance;
}

void Camera::Rotate(float angle) noexcept
{
	//upVector = glm::rotate(angle, upVector);
	//rightVector = rotate_by(angle, rightVector);
}

void Camera::ResetUp(glm::vec3 startUpPosition)
{
	this->upVector.x = startUpPosition.x;
	this->upVector.y = startUpPosition.y;
	this->upVector.z = startUpPosition.z;
	rightVector = { startUpPosition.y, -startUpPosition.x, startUpPosition.z };
}

void Camera::SetViewSize(int width, int height) noexcept
{
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	//aspectRatio = aspectRatio > 1.0f ? aspectRatio : 1.0f / aspectRatio;
}

void Camera::SetZoom(float amount) noexcept
{
	zoom = glm::clamp(amount, 0.1f, 100.0f);
}
