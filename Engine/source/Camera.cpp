#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include "Camera.hpp"
#include "Engine.hpp"

void Camera::Update()
{
	//glm::vec3 frontT;
	//frontT.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	//frontT.y = sin(glm::radians(pitch));
	//frontT.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	//front = glm::normalize(frontT) * glm::rotateZ(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(rotate2D))
	switch (cameraType)
	{
	case CameraType::TwoDimension:
		glm::vec2 wSize = Engine::Instance().GetWindow()->GetWindowSize();
		view =  glm::translate(glm::mat4(1.0f), glm::vec3(-cameraPosition.x / wSize.x * 2.f, -cameraPosition.y / wSize.y * 2.f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(rotate2D), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
		projection = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, -1.f, 1.f);
		//projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
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
		std::cout << cameraPosition.x << '\n';
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
	cameraViewSize = { width, height };
	//aspectRatio = aspectRatio > 1.0f ? aspectRatio : 1.0f / aspectRatio;
}

void Camera::SetZoom(float amount) noexcept
{
	zoom = glm::clamp(amount, nearClip, farClip);
}
