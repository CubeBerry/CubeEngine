//Author: DOYEONG LEE
//Project: CubeEngine
//File: Ball.cpp
#include "PocketBallDemo/Ball.hpp"

#include <iostream>
Ball::Ball(glm::vec3 pos_, glm::vec3 size_, std::string name, ObjectType objectType)
	: Object(pos_, size_, name, objectType)
{
}

void Ball::Init()
{
}

void Ball::Update(float dt)
{
	Object::Update(dt);
}

void Ball::End()
{
}