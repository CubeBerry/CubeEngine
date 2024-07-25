//Author: DOYEONG LEE
//Project: CubeEngine
//File: Camera.cpp
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include "Camera.hpp"
#include "Engine.hpp"

void Camera::Update()
{
	glm::vec2 wSize = Engine::Instance().GetWindow().GetWindowSize();
	switch (cameraType)
	{
	case CameraType::TwoDimension:
		view = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraPosition.x * 2.f, -cameraPosition.y * 2.f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotate2D), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
		projection = glm::ortho(-cameraViewSize.x, cameraViewSize.x, -cameraViewSize.y, cameraViewSize.y, -1.f, 1.f);
		break;
	case CameraType::ThreeDimension:
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		switch (Engine::GetRenderManager()->GetGraphicsMode())
		{
		case GraphicsMode::GL:
			direction.y = sin(glm::radians(-pitch));
			break;
		case GraphicsMode::VK:
			direction.y = sin(glm::radians(pitch));
			break;
		}
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		front = glm::normalize(direction);
		right = glm::normalize(glm::cross(direction, worldUp));
		up = glm::normalize(glm::cross(right, front));

		view = glm::lookAt(cameraPosition, cameraPosition + front, up);
		projection = glm::perspective(glm::radians(zoom), static_cast<float>(wSize.x) / static_cast<float>(wSize.y), nearClip, farClip);
		break;
	default:
		break;
	}
}

void Camera::SetCenter(glm::vec3 centerPosition) noexcept
{
	cameraTarget = centerPosition;
}

void Camera::SetCameraPosition(glm::vec3 cameraPosition_) noexcept
{
	cameraPosition = cameraPosition_;
}

void Camera::MoveCameraPos(CameraMoveDir dir, float speed)
{
	switch (dir)
	{
	case CameraMoveDir::FOWARD:
		if (cameraType == CameraType::ThreeDimension)
		{
			cameraPosition += front * speed;
		}
		break;
	case CameraMoveDir::BACKWARD:
		if (cameraType == CameraType::ThreeDimension)
		{
			cameraPosition -= front * speed;
		}
		break;
	case CameraMoveDir::UP:
		switch (cameraType)
		{
		case CameraType::TwoDimension:
			cameraPosition += normalize(up) * speed;
			break;
		case CameraType::ThreeDimension:
			cameraPosition += up * speed;
			break;
		}
		break;
	case CameraMoveDir::DOWN:
		switch (cameraType)
		{
		case CameraType::TwoDimension:
			cameraPosition -= normalize(up) * speed;
			break;
		case CameraType::ThreeDimension:
			cameraPosition -= up * speed;
			break;
		}
		break;
	case CameraMoveDir::LEFT:
		switch (cameraType)
		{
		case CameraType::TwoDimension:
			cameraPosition -= normalize(right) * speed;
			break;
		case CameraType::ThreeDimension:
			cameraPosition -= right * speed;
			break;
		}
		break;
	case CameraMoveDir::RIGHT:
		switch (cameraType)
		{
		case CameraType::TwoDimension:
			cameraPosition += normalize(right) * speed;
			break;
		case CameraType::ThreeDimension:
			cameraPosition += right * speed;
			break;
		}
		break;
	}
}

void Camera::UpdaetCameraDirectrion(glm::vec2 dir)
{
	yaw += dir.x * cameraSensitivity;
	pitch += dir.y * cameraSensitivity;

	pitch = glm::clamp(pitch, -89.f, 89.f);
	Update();
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

void Camera::Reset(glm::vec3 startUpPosition)
{
	up = startUpPosition;
	right = { startUpPosition.y, -startUpPosition.x, startUpPosition.z };
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	SetCenter({ 0.f,0.f,0.f });
	SetZoom(1.f);
	pitch = -90.f;
	yaw = 0.f;
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
