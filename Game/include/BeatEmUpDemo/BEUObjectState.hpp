//Author: DOYEONG LEE
//Project: CubeEngine
//File: BEUObjectState.hpp
#pragma once
enum class BEUObjectStates
{
	DIRECTION = 1, // off : LEFT, on : RIGHT
	MOVE = 2,
	ATTACK = 4,
	JUMPING = 8,
	FALLING = 16,
	HIT = 32,
	KNOCKBACK = 64,
	LAYING = 128,
	MOVEFOWARD = 256,
};