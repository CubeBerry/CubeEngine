//Author: DOYEONG LEE
//Project: CubeEngine
//File: InputManager.cpp
#include "InputManager.hpp"
#include "Engine.hpp"

void InputManager::InputPollEvent(SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_KEYDOWN:
		KeyDown(static_cast<KEYBOARDKEYS>(event.key.keysym.sym));
		break;
	case SDL_KEYUP:
		KeyUp(static_cast<KEYBOARDKEYS>(event.key.keysym.sym));
		break;
	case SDL_MOUSEBUTTONDOWN:
		MouseButtonDown(static_cast<MOUSEBUTTON>(event.button.button), event.button.x, event.button.y);
		break;
	case SDL_MOUSEBUTTONUP:
		MouseButtonUp(static_cast<MOUSEBUTTON>(event.button.button), event.button.x, event.button.y);
		break;
	default:
		break;
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

bool InputManager::IsKeyPressedOnce(KEYBOARDKEYS keycode)
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

bool InputManager::IsMouseButtonPressedOnce(MOUSEBUTTON button)
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