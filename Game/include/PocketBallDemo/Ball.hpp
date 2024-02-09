//Author: DOYEONG LEE
//Project: CubeEngine
//File: Ball.hpp
#pragma once
#include "Object.hpp"


class Ball : public Object
{
public:
    Ball() = default;
	Ball(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType);
	~Ball() { End(); }

	void Init() override;
	void Update(float dt) override;
	//void Draw(float dt) override;
	void End();
private:
};