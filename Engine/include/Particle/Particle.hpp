//Author: DOYEONG LEE
//Project: CubeEngine
//File: Particle.cpp
#pragma once
#include <string>
#include <glm/vec4.hpp>
#include "BasicComponents/DynamicSprite.hpp"

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
    Particle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, glm::vec3 angle_, float lifeTime_, ParticleType type = ParticleType::REC, std::string spriteName_ = "", glm::vec4 color_ = { 1.f,1.f,1.f,1.f }, ParticleEffect particE = ParticleEffect::NORMAL);
    ~Particle();

    void Update(float dt);
    bool inUse() const { return lifeTime > 0; }

    void SetFadeOutAmount(float amount) { fadeOutAmount = amount; }
    DynamicSprite* GetSprite() { return sprite; }
    float GetLifetime() { return lifeTime; }

    void SetGravity(float g, bool on = true) { gravity = g; useGravity = on; }
    void SetDrag(float d) { drag = d; }
private:
    glm::vec3 position = { 0.f, 0.f, 0.f };
    glm::vec3 velocity = { 0.f, 0.f, 0.f };
    glm::vec3 size     = { 0.f, 0.f, 0.f };
    glm::vec3 angle    = { 0.f, 0.f, 0.f };

    // Self-contained physics state.
    float gravity    = 9.8f;
    bool  useGravity = false;
    float drag       = 0.f;   // velocity damping per second (0 = no drag)

    float fadeOutAmount = 0.1f;

    glm::vec4 color = { 1.f,1.f,1.f,1.f };

    float lifeTime = 0;
    ParticleType particleType = ParticleType::REC;
    std::string spriteName = "";

    DynamicSprite* sprite = nullptr;
    ParticleEffect effect;
};