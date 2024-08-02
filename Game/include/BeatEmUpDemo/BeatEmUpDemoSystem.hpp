//Author: DOYEONG LEE
//Project: CubeEngine
//File: BeatEmUpDemoSystem.hpp
#pragma once

#include <vector>

class BeatEmUpDemoSystem
{
public:
	BeatEmUpDemoSystem() {}
	~BeatEmUpDemoSystem() = default;

	void Init();
	void Update(float dt);
	void End();

	void HpDecrease(float damage) { hp -= damage; }
#ifdef _DEBUG
#endif
protected:
	float maxHp = 100.f;
	float hp = 100.f;
};