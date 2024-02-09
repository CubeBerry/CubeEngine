//Author: DOYEONG LEE
//Project: CubeEngine
//File: ParticleManager.cpp
#include "particle/ParticleManager.hpp"


ParticleManager::~ParticleManager()
{
	Clear();
}

void ParticleManager::AddRandomParticle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime, int particleAmount, glm::vec4 color_, ParticleType type, std::string spriteName_, bool isFade)
{
	for (int i = 0; i < particleAmount; i++) // loop for generate particle
	{
		glm::vec3 newVel = { speed_.x + rand() % 20 - rand() % 20 , speed_.y + rand() % 20 - rand() % 20, 0.f };
		float tempVal = static_cast<float>(rand() % 20 - rand() % 20);
		glm::vec3 newSize = { size_.x + tempVal, size_.y + tempVal, 0.f };
		float newAngle = { angle_ + rand() % 1 - rand() % 360 };
		if (isFade == false)
		{
			AddSingleParticle(position_, newSize, newVel, newAngle, lifeTime, color_, type, spriteName_);
		}
		else
		{
			AddSingleFadeOutParticle(position_, newSize, newVel, newAngle, 1.f, color_, type, spriteName_, 0.5f);
		}
	}
}

void ParticleManager::AddSingleParticle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime, glm::vec4 color_, ParticleType type, std::string spriteName_)
{
	//float newAngle = { angle_ + rand() % 1 - rand() % 360 };
	particles.push_back(std::move(Particle(position_, size_, speed_, angle_, lifeTime, type, spriteName_, color_)));
}

void ParticleManager::AddSingleFadeOutParticle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime, glm::vec4 color_, ParticleType type, std::string spriteName_, float decreaseAmount)
{
	particles.push_back(std::move(Particle(position_, size_, speed_, angle_, lifeTime, type, spriteName_, color_, ParticleEffect::FADEOUT)));
	GetLastParticle()->SetFadeOutAmount(decreaseAmount);
}

void ParticleManager::Update(float dt)
{
	for (int i = 0; i < particles.size(); i++)
	{
		if (particles.at(i).inUse() == true) // check particle life time is still left
		{
			particles.at(i).Update(dt);
		}
		else
		{
			delete (particles.begin() + i)->GetSprite();
			particles.erase(particles.begin() + i);
		}
	}
}

void ParticleManager::Clear()
{
	for (int i = 0; i < particles.size(); i++)
	{
		delete (particles.begin() + i)->GetSprite();
	}
	particles.clear();
}
