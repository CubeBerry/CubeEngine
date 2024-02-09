//Author: DOYEONG LEE
//Project: CubeEngine
//File: Particle.cpp
#pragma once
#include <string>
#include "glm/glm.hpp"
#include "BasicComponents/Physics2D.hpp"
#include "BasicComponents/Sprite.hpp"

enum class ParticleType
{
    REC,
    SPRI,
    ANIMESPRI
};

enum class ParticleEffect
{
    NORMAL,
    FADEOUT
};

class Particle
{
public:
    Particle() = default;
    Particle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime_, ParticleType type = ParticleType::REC, std::string spriteName_ = "", glm::vec4 color_ = {1.f,1.f,1.f,1.f}, ParticleEffect particE = ParticleEffect::NORMAL);
    ~Particle();

    void Update(float dt);
    bool inUse() const { return lifeTime > 0; }

    void SetFadeOutAmount(float amount) { fadeOutAmount = amount; }
    Sprite* GetSprite() { return sprite; }
    float GetLifetime() { return lifeTime; }
    Physics2D* GetPhysics() { return &pPhysics; }
private:
    glm::vec3 position = { 0.f, 0.f, 0.f };
    glm::vec3 speed = { 0.f, 0.f, 0.f };
    glm::vec3 size = { 0.f, 0.f, 0.f };
    float angle = 0.f;

    float fadeOutAmount = 0.1f;

    glm::vec4 color = { 1.f,1.f,1.f,1.f };

    float lifeTime = 0;
    ParticleType particleType = ParticleType::REC;
    std::string spriteName = "";

    Sprite* sprite = nullptr;
    ParticleEffect effect;
    Physics2D pPhysics;
};