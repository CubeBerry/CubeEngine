//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: Camera.cpp
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Camera.hpp"
#include "Engine.hpp"

Camera::~Camera()
{
	Engine::GetLogger().LogDebug(LogCategory::Engine, "Camera [" + name + "] Deleted");
}

void Camera::Update()
{
	// @TODO Why only fixed Window size work for projection matrix even if window is resized?
	// @TODO Need to acknowledge the difference between resizing and stretching/squashing
	glm::vec2 wSize = Engine::GetWindow().GetWindowSize();
	switch (cameraType)
	{
	case CameraType::Orthographic:
		if (isThirdPersonView == true)
		{
			view = glm::rotate(glm::mat4(1.0f), -glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::translate(glm::mat4(1.0f), -cameraCenter);
		}
		else
		{
			view = glm::rotate(glm::mat4(1.0f), -glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::translate(glm::mat4(1.0f), -cameraPosition);
		}

		switch (Engine::GetRenderManager()->GetGraphicsMode())
		{
		case GraphicsMode::GL:
			projection = glm::orthoRH_NO(-cameraViewSize.x / zoom, cameraViewSize.x / zoom, -cameraViewSize.y / zoom, cameraViewSize.y / zoom, nearClip, farClip);
			break;
		case GraphicsMode::VK:
			projection = glm::orthoRH_ZO(-cameraViewSize.x / zoom, cameraViewSize.x / zoom, -cameraViewSize.y / zoom, cameraViewSize.y / zoom, nearClip, farClip);
			// Flip y-axis for Vulkan
			projection[1][1] *= -1.0f;
			break;
		case GraphicsMode::DX:
			projection = glm::orthoRH_ZO(-cameraViewSize.x / zoom, cameraViewSize.x / zoom, -cameraViewSize.y / zoom, cameraViewSize.y / zoom, nearClip, farClip);
			break;
		}
		break;
	case CameraType::Perspective:
		glm::vec3 direction;
		glm::vec3 desiredPosition = cameraCenter + cameraOffset;

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));

		switch (Engine::GetRenderManager()->GetGraphicsMode())
		{
		case GraphicsMode::GL:
			direction.y = sin(glm::radians(-pitch));
			break;
		case GraphicsMode::VK:
			direction.y = sin(glm::radians(-pitch));
			break;
		case GraphicsMode::DX:
			direction.y = sin(glm::radians(-pitch));
			break;
		}

		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		back = glm::normalize(direction);

		if (isThirdPersonView == true)
		{
			right = glm::normalize(glm::cross(back, worldUp));
			up = glm::normalize(glm::cross(right, back));

			if (roll != 0.0f)
			{
				glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f), glm::radians(roll), back);
				up = glm::vec3(rollMat * glm::vec4(up, 0.0f));
				right = glm::vec3(rollMat * glm::vec4(right, 0.0f));
			}

			cameraPosition = desiredPosition - back * cameraDistance;
		}
		else
		{
			right = glm::normalize(glm::cross(direction, worldUp));
			up = glm::normalize(glm::cross(right, back));

			if (roll != 0.0f)
			{
				glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f), glm::radians(roll), back);
				up = glm::vec3(rollMat * glm::vec4(up, 0.0f));
				right = glm::vec3(rollMat * glm::vec4(right, 0.0f));
			}
		}

		switch (Engine::GetRenderManager()->GetGraphicsMode())
		{
		case GraphicsMode::GL:
			if (isThirdPersonView == true)
			{
				view = glm::lookAt(cameraPosition, cameraCenter, up);
				//view = glm::lookAt(cameraPosition, cameraCenter + back, up);
			}
			else
			{
				view = glm::lookAt(cameraPosition, cameraPosition + back, up);
			}
			projection = glm::perspectiveRH_NO(glm::radians(baseFov / log2(zoom + 1.0f)), static_cast<float>(wSize.x) / static_cast<float>(wSize.y), nearClip, farClip);
			break;

		case GraphicsMode::VK:
			if (isThirdPersonView == true)
			{
				view = glm::lookAt({ cameraPosition.x, -cameraPosition.y, cameraPosition.z }, { cameraCenter.x, -cameraCenter.y, cameraCenter.z }, up);
				//view = glm::lookAt({ cameraPosition.x, -cameraPosition.y, cameraPosition.z }, cameraCenter + back, up);
			}
			else
			{
				view = glm::lookAt(cameraPosition, cameraPosition + back, up);
				//view = glm::lookAt({ cameraPosition.x, -cameraPosition.y, cameraPosition.z }, glm::vec3{ cameraPosition.x, -cameraPosition.y, cameraPosition.z } + back, up);
			}
			//projection = glm::perspectiveRH_ZO(glm::radians(baseFov / log2(zoom + 1.0f)), static_cast<float>(wSize.x) / static_cast<float>(wSize.y), nearClip, farClip);
			projection = glm::perspectiveRH_ZO(glm::radians(baseFov / log2(zoom + 1.0f)), wSize.x / wSize.y, nearClip, farClip);
			// Flip y-axis for Vulkan
			projection[1][1] *= -1.0f;
			break;

		case GraphicsMode::DX:
			if (isThirdPersonView == true)
			{
				view = glm::lookAtRH({ cameraPosition.x, -cameraPosition.y, cameraPosition.z }, { cameraCenter.x, -cameraCenter.y, cameraCenter.z }, up);
				//view = glm::lookAt({ cameraPosition.x, -cameraPosition.y, cameraPosition.z }, cameraCenter + back, up);
			}
			else
			{
				view = glm::lookAtRH(cameraPosition, cameraPosition + back, up);
				//view = glm::lookAt({ cameraPosition.x, -cameraPosition.y, cameraPosition.z }, glm::vec3{ cameraPosition.x, -cameraPosition.y, cameraPosition.z } + back, up);
			}
			//projection = glm::perspectiveRH_ZO(glm::radians(baseFov / log2(zoom + 1.0f)), static_cast<float>(wSize.x) / static_cast<float>(wSize.y), nearClip, farClip);
			projection = glm::perspectiveRH_ZO(glm::radians(baseFov / log2(zoom + 1.0f)), wSize.x / wSize.y, nearClip, farClip);
			break;
		}
		break;

	default:
		break;
	}
}

void Camera::SetTarget(glm::vec3 pos)
{
	switch (cameraType)
	{
	case CameraType::Orthographic:
		cameraPosition = pos;
		cameraCenter = pos;
		break;
	case CameraType::Perspective:
		if (isThirdPersonView)
		{
			cameraCenter = pos;
		}
		else
		{
			LookAt(pos);
			break;
		}
	}
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
		if (cameraType == CameraType::Perspective)
		{
			cameraPosition += back * speed;
		}
		else if (cameraType == CameraType::Orthographic)
		{
			cameraPosition.z += speed;
		}
		break;
	case CameraMoveDir::BACKWARD:
		if (cameraType == CameraType::Perspective)
		{
			cameraPosition -= back * speed;
		}
		else if (cameraType == CameraType::Orthographic)
		{
			cameraPosition.z -= speed;
		}
		break;
	case CameraMoveDir::UP:
		switch (cameraType)
		{
		case CameraType::Orthographic:
			cameraPosition += normalize(up) * speed;
			break;
		case CameraType::Perspective:
			cameraPosition += up * speed;
			break;
		}
		break;
	case CameraMoveDir::DOWN:
		switch (cameraType)
		{
		case CameraType::Orthographic:
			cameraPosition -= normalize(up) * speed;
			break;
		case CameraType::Perspective:
			cameraPosition -= up * speed;
			break;
		}
		break;
	case CameraMoveDir::LEFT:
		switch (cameraType)
		{
		case CameraType::Orthographic:
			cameraPosition -= normalize(right) * speed;
			break;
		case CameraType::Perspective:
			cameraPosition -= right * speed;
			break;
		}
		break;
	case CameraMoveDir::RIGHT:
		switch (cameraType)
		{
		case CameraType::Orthographic:
			cameraPosition += normalize(right) * speed;
			break;
		case CameraType::Perspective:
			cameraPosition += right * speed;
			break;
		}
		break;
	}
}

void Camera::UpdateCameraDirection(glm::vec2 dir)
{
	yaw += dir.x * cameraSensitivity;
	pitch += dir.y * cameraSensitivity;

	pitch = glm::clamp(pitch, -89.f, 89.f);
	yaw = fmod(yaw, 360.0f);
	if (yaw < 0.0f)
	{
		yaw += 360.0f;
	}
	Update();
}

Ray Camera::CalculateRayFrom2DPosition(glm::vec2 pos)
{
	float x = (2.0f * pos.x) / cameraViewSize.x;
	float y = -(2.0f * pos.y) / cameraViewSize.y;
	glm::vec4 rayClip = glm::vec4(x, y, -1.0, 1.0);

	glm::vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

	glm::vec4 rayWorld = glm::inverse(view) * rayEye;
	glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

	glm::vec3 cameraPos = glm::vec3(glm::inverse(view)[3]);

	return Ray{ cameraPos, rayDirection };
}

void Camera::RotateOrthographic(float angle) noexcept
{
	switch (cameraType)
	{
	case CameraType::Orthographic:
		roll = angle;
		break;
	case CameraType::Perspective:
		break;
	default:
		break;
	}
}

void Camera::LookAt(glm::vec3 pos)
{
	if (cameraType == CameraType::Perspective)
	{
		cameraCenter = pos;
		glm::vec3 direction = glm::normalize(cameraCenter - cameraPosition);

		pitch = glm::degrees(asin(-direction.y));
		yaw = glm::degrees(atan2(direction.z, direction.x));

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		yaw = glm::mod(yaw, 360.0f);
		if (yaw < 0.0f)
		{
			yaw += 360.0f;
		}

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		switch (Engine::GetRenderManager()->GetGraphicsMode())
		{
		case GraphicsMode::GL:
			direction.y = sin(glm::radians(-pitch));
			break;
		case GraphicsMode::VK:
			direction.y = sin(glm::radians(-pitch));
			break;
		case GraphicsMode::DX:
			direction.y = sin(glm::radians(-pitch));
			break;
		}
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		back = glm::normalize(direction);

		right = glm::normalize(glm::cross(back, worldUp));
		up = glm::normalize(glm::cross(right, back));

		switch (Engine::GetRenderManager()->GetGraphicsMode())
		{
		case GraphicsMode::GL:
			view = glm::lookAt(cameraPosition, cameraCenter, up);
			break;
		case GraphicsMode::VK:
			view = glm::lookAt({ cameraPosition.x, cameraPosition.y, cameraPosition.z }, { cameraCenter.x, cameraCenter.y, cameraCenter.z }, up);
			break;
		case GraphicsMode::DX:
			view = glm::lookAt({ cameraPosition.x, cameraPosition.y, cameraPosition.z }, { cameraCenter.x, cameraCenter.y, cameraCenter.z }, up);
			break;
		}
	}
}

void Camera::Reset()
{
	up = { 0.0f, 1.0f, 0.0f };
	right = { 1.0f, 0.0f, 0.0f };
	back = { 0.0f, 0.0f, -1.0f };
	cameraPosition = { 0.f,0.f,0.f };
	cameraCenter = { 0.f,0.f,0.f };
	SetZoom(1.f);
	pitch = 00.f;
	yaw = -90.f;
	roll = 0.0f;
	nearClip = 0.1f;
	farClip = 1000.0f;
	baseFov = 45.f;
	cameraSensitivity = 1.f;
	isThirdPersonView = false;
}

void Camera::SetViewSize(int width, int height) noexcept
{
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	cameraViewSize = { width, height };
	//aspectRatio = aspectRatio > 1.0f ? aspectRatio : 1.0f / aspectRatio;
}

void Camera::SetZoom(float amount) noexcept
{
	zoom = glm::clamp(amount, 0.001f, 10000.0f);
}
