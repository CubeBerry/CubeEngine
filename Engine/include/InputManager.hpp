//Author: DOYEONG LEE
//Project: CubeEngine
//File: InputManager.hpp
#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>
#include "glm/vec2.hpp"

#include <iostream>

enum class MOUSEBUTTON
{
	LEFT   =  1,
	MIDDLE =  2,
	RIGHT  =  3
};

enum class KEYBOARDKEYS
{
	UNKNOWN = 0,

	RETURN = '\r',
	ESCAPE = '\x1B',
	BACKSPACE = '\b',
	TAB = '\t',
	SPACE = ' ',
	EXCLAIM = '!',
	QUOTEDBL = '"',
	HASH = '#',
	PERCENT = '%',
	DOLLAR = '$',
	AMPERSAND = '&',
	QUOTE = '\'',
	LEFTPAREN = '(',
	RIGHTPAREN = ')',
	ASTERISK = '*',
	PLUS = '+',
	COMMA = ',',
	MINUS = '-',
	PERIOD = '.',
	SLASH = '/',
	NUMBER_0 = '0',
	NUMBER_1 = '1',
	NUMBER_2 = '2',
	NUMBER_3 = '3',
	NUMBER_4 = '4',
	NUMBER_5 = '5',
	NUMBER_6 = '6',
	NUMBER_7 = '7',
	NUMBER_8 = '8',
	NUMBER_9 = '9',
	COLON = ':',
	SEMICOLON = ';',
	LESS = '<',
	EQUALS = '=',
	GREATER = '>',
	QUESTION = '?',
	AT = '@',

	/*
	   Skip uppercase letters
	 */

	 LEFTBRACKET = '[',
	 BACKSLASH = '\\',
	 RIGHTBRACKET = ']',
	 CARET = '^',
	 UNDERSCORE = '_',
	 BACKQUOTE = '`',
	 A = 'a',
	 B = 'b',
	 C = 'c',
	 D = 'd',
	 E = 'e',
	 F = 'f',
	 G = 'g',
	 H = 'h',
	 I = 'i',
	 J = 'j',
	 K = 'k',
	 L = 'l',
	 M = 'm',
	 N = 'n',
	 O = 'o',
	 P = 'p',
	 Q = 'q',
	 R = 'r',
	 S = 's',
	 T = 't',
	 U = 'u',
	 V = 'v',
	 W = 'w',
	 X = 'x',
	 Y = 'y',
	 Z = 'z',

	 CAPSLOCK = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK),

	 F1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1),
	 F2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2),
	 F3 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3),
	 F4 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4),
	 F5 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5),
	 F6 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6),
	 F7 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7),
	 F8 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8),
	 F9 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9),
	 F10 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10),
	 F11 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11),
	 F12 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12),

	 PRINTSCREEN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN),
	 SCROLLLOCK = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK),
	 PAUSE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE),
	 INSERT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT),
	 HOME = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME),
	 PAGEUP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP),
	 DELETE_KEY = '\x7F',
	 UNLOAD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END),
	 PAGEDOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN),
	 RIGHT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
	 LEFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
	 DOWN = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
	 UP = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP),

	 NUMLOCKCLEAR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR),
	 KP_DIVIDE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
	 KP_MULTIPLY = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
	 KP_MINUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
	 KP_PLUS = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
	 KP_ENTER = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER),
	 KP_1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1),
	 KP_2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2),
	 KP_3 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3),
	 KP_4 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4),
	 KP_5 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5),
	 KP_6 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6),
	 KP_7 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7),
	 KP_8 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8),
	 KP_9 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9),
	 KP_0 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0),
	 KP_PERIOD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),

	 LCTRL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL),
	 LSHIFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT),
	 LALT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT),
	 LGUI = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI),
	 RCTRL = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL),
	 RSHIFT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT),
	 RALT = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT),
	 RGUI = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI),

	 MODE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE),
};

// Immutable snapshot of input state captured at the start of each frame.
// All query functions are const — safe to call from any thread.
struct InputSnapshot
{
	std::unordered_map<KEYBOARDKEYS, bool> keyStates;
	std::unordered_map<KEYBOARDKEYS, bool> keyStatePrev;
	std::unordered_map<MOUSEBUTTON, bool>  mouseStates;
	std::unordered_map<MOUSEBUTTON, bool>  mouseStatePrev;
	glm::vec2 mousePosition      = { 0.f, 0.f };
	glm::vec2 mouseWheel         = { 0.f, 0.f };
	glm::vec2 relativeMouseState = { 0.f, 0.f };

	bool IsKeyPressed(KEYBOARDKEYS key) const
	{
		auto it = keyStates.find(key);
		return it != keyStates.end() && it->second;
	}

	bool IsKeyPressOnce(KEYBOARDKEYS key) const
	{
		auto itCur  = keyStates.find(key);
		auto itPrev = keyStatePrev.find(key);
		bool cur  = (itCur  != keyStates.end())    && itCur->second;
		bool prev = (itPrev != keyStatePrev.end()) && itPrev->second;
		return cur && !prev;
	}

	bool IsKeyReleaseOnce(KEYBOARDKEYS key) const
	{
		auto itCur  = keyStates.find(key);
		auto itPrev = keyStatePrev.find(key);
		bool cur  = (itCur  != keyStates.end())    && itCur->second;
		bool prev = (itPrev != keyStatePrev.end()) && itPrev->second;
		return !cur && prev;
	}

	bool IsMouseButtonPressed(MOUSEBUTTON btn) const
	{
		auto it = mouseStates.find(btn);
		return it != mouseStates.end() && it->second;
	}

	bool IsMouseButtonPressOnce(MOUSEBUTTON btn) const
	{
		auto itCur  = mouseStates.find(btn);
		auto itPrev = mouseStatePrev.find(btn);
		bool cur  = (itCur  != mouseStates.end())    && itCur->second;
		bool prev = (itPrev != mouseStatePrev.end()) && itPrev->second;
		return cur && !prev;
	}

	bool IsMouseButtonReleaseOnce(MOUSEBUTTON btn) const
	{
		auto itCur  = mouseStates.find(btn);
		auto itPrev = mouseStatePrev.find(btn);
		bool cur  = (itCur  != mouseStates.end())    && itCur->second;
		bool prev = (itPrev != mouseStatePrev.end()) && itPrev->second;
		return !cur && prev;
	}

	glm::vec2 GetMousePosition() const { return mousePosition; }
	glm::vec2 GetMouseWheelMotion() const { return mouseWheel; }
	glm::vec2 GetRelativeMouseState() const { return relativeMouseState; }
};

class InputManager
{
public:
	InputManager();
	~InputManager();

	void InputPollEvent(SDL_Event& event);

	bool IsKeyPressed(KEYBOARDKEYS keycode);
	bool IsKeyPressOnce(KEYBOARDKEYS keycode);
	bool IsKeyReleaseOnce(KEYBOARDKEYS keycode);

	bool IsMouseButtonPressed(MOUSEBUTTON button);
	bool IsMouseButtonPressOnce(MOUSEBUTTON button);
	bool IsMouseButtonReleaseOnce(MOUSEBUTTON button);

	glm::vec2 GetMousePosition();
	glm::vec2 GetMouseWheelMotion();
	void ResetWheelMotion();

	void SetRelativeMouseMode(bool state);
	bool GetRelativeMouseMode();
	glm::vec2 GetRelativeMouseState();

	// Batch update previous state at frame boundary (call before CreateSnapshot)
	void UpdatePreviousState();

	// Create an immutable snapshot of current input state for thread-safe reading
	InputSnapshot CreateSnapshot();

protected:
	void KeyDown(KEYBOARDKEYS keycode)
	{
		SetKeyState(keycode, true);
	}

	void KeyUp(KEYBOARDKEYS keycode)
	{
		SetKeyState(keycode, false);
	}

	void MouseButtonDown(MOUSEBUTTON button, float /*x*/, float /*y*/)
	{
		SetMouseButtonState(button, true);
	}

	void MouseButtonUp(MOUSEBUTTON button, float /*x*/, float /*y*/)
	{
		SetMouseButtonState(button, false);
	}

	void SetKeyState(KEYBOARDKEYS keycode, bool state)
	{
		keyStates[keycode] = state;
	}

	void SetMouseButtonState(MOUSEBUTTON button, bool state)
	{
		mouseButtonStates[button] = state;
	}
	void MouseWheel(const SDL_Event& event) 
	{
		mouseWheelMotion.x = static_cast<float>(event.wheel.x);
		mouseWheelMotion.y = static_cast<float>(event.wheel.y);
	}
private:
	std::unordered_map<KEYBOARDKEYS, bool> keyStates;
	std::unordered_map<KEYBOARDKEYS, bool> keyStatePrev;

	std::unordered_map<MOUSEBUTTON, bool> mouseButtonStates;
	std::unordered_map<MOUSEBUTTON, bool> mouseButtonStatePrev;

	glm::vec2 mouseWheelMotion = glm::vec2(0.0f, 0.0f);
	float mouseRelX = 0;
	float mouseRelY = 0;
};