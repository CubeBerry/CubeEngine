//Author: DOYEONG LEE
//Project: CubeEngine
//File: InputManager.cpp
#include "InputManager.hpp"
#include "Engine.hpp"

InputManager::InputManager() {}

InputManager::~InputManager() {}

void InputManager::InputPollEvent(SDL_Event& event)
{
	mouseWheelMotion = { 0.f,0.f };
	switch (event.type)
	{
	case SDL_KEYDOWN:
		KeyDown(static_cast<KEYBOARDKEYS>(event.key.keysym.sym));
		break;
	case SDL_KEYUP:
		KeyUp(static_cast<KEYBOARDKEYS>(event.key.keysym.sym));
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (!(ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered()))
		{
			MouseButtonDown(static_cast<MOUSEBUTTON>(event.button.button), event.button.x, event.button.y);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (!(ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered()))
		{
			MouseButtonUp(static_cast<MOUSEBUTTON>(event.button.button), event.button.x, event.button.y);
		}
		break;
	case SDL_MOUSEWHEEL:
		if (!(ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered()))
		{
			MouseWheel(event);
		}
		break;
	case SDL_MOUSEMOTION:
		mouseRelX = event.motion.xrel;
		mouseRelY = event.motion.yrel;
		break;
	default:
		break;
	}
	if (event.type == SDL_MOUSEMOTION)
	{
		mouseRelX = event.motion.xrel;
		mouseRelY = event.motion.yrel;
	}
}

bool InputManager::IsKeyPressed(KEYBOARDKEYS keycode)
{
	auto it = keyStates.find(keycode);
	if (it != keyStates.end())
	{
		keyStatePrev[keycode] = it->second;
		return it->second;
	}
	return false;
}

bool InputManager::IsKeyPressOnce(KEYBOARDKEYS keycode)
{
	auto it = keyStates.find(keycode);
	if (it != keyStates.end())
	{
		bool isPressed = it->second && !keyStatePrev[keycode];
		keyStatePrev[keycode] = it->second;
		return isPressed;
	}
	return false;
}

bool InputManager::IsKeyReleaseOnce(KEYBOARDKEYS keycode)
{
	auto it = keyStates.find(keycode);
	if (it != keyStates.end())
	{
		bool isReleased = !it->second && keyStatePrev[keycode];
		keyStatePrev[keycode] = it->second;
		return isReleased;
	}
	return false;
}

bool InputManager::IsMouseButtonPressed(MOUSEBUTTON button)
{
	auto it = mouseButtonStates.find(button);
	if (it != mouseButtonStates.end())
	{
		mouseButtonStatePrev[button] = it->second;
		return it->second;
	}
	return false;
}

bool InputManager::IsMouseButtonPressOnce(MOUSEBUTTON button)
{
	auto it = mouseButtonStates.find(button);
	if (it != mouseButtonStates.end())
	{
		bool isPressed = it->second && !mouseButtonStatePrev[button];
		mouseButtonStatePrev[button] = it->second;
		return isPressed;
	}
	return false;
}

bool InputManager::IsMouseButtonReleaseOnce(MOUSEBUTTON button)
{
	auto it = mouseButtonStates.find(button);
	if (it != mouseButtonStates.end())
	{
		bool isReleased = !it->second && mouseButtonStates[button];
		mouseButtonStatePrev[button] = it->second;
		return isReleased;
	}
	return false;
}

glm::vec2 InputManager::GetMousePosition()
{
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	glm::vec2 pos = { mouseX, mouseY };
	glm::vec2 windowViewSize = Engine::Instance().GetCameraManager().GetViewSize();
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(Engine::Instance().GetWindow().GetWindow(), &w, &h);
	float zoom = Engine::Instance().GetCameraManager().GetZoom();

	pos = { windowViewSize.x / 2.f * (pos.x / (static_cast<float>(w) / 2.f) - 1) / zoom, windowViewSize.y / 2.f * (pos.y / (static_cast<float>(h) / 2.f) - 1) / zoom };
	return pos;
}

glm::vec2 InputManager::GetMouseWheelMotion()
{
	return mouseWheelMotion;
}

void InputManager::SetRelativeMouseMode(bool state)
{
	if (state == true)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	mouseRelX = 0;
	mouseRelY = 0;
}

bool InputManager::GetRelativeMouseMode()
{
	return SDL_GetRelativeMouseMode();
}

glm::vec2 InputManager::GetRelativeMouseState()
{
	glm::vec2 temp{ mouseRelX, mouseRelY };
	mouseRelX = 0;
	mouseRelY = 0;
	return temp;
}