//Author: DOYEONG LEE
//Project: CubeEngine
//File: ParticleManager.hpp
#pragma once
#include <vector>

#include "Particle.hpp"

class ParticleManager
{
public:
	ParticleManager() = default;
    ~ParticleManager();

	//For Test
    void AddRandomParticle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime, int particleAmount, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, ParticleType type = ParticleType::REC, std::string spriteName_ = "", bool isFade = false);
	//For Test

	void AddSingleParticle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, ParticleType type = ParticleType::REC, std::string spriteName_ = "");
	void AddSingleFadeOutParticle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime, glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, ParticleType type = ParticleType::REC, std::string spriteName_ = "", float decreaseAmount = 0.1f);

	void Update(float dt);
	void Clear();
	Particle* GetLastParticle() { return &particles.at(particles.size() - 1); }
private:
	std::vector<Particle> particles;
};