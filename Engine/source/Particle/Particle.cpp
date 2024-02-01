//Author: DOYEONG LEE
//Project: CubeEngine
//File: Particle.hpp
#include "Particle/Particle.hpp"
#include "Engine.hpp"

Particle::Particle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime_, ParticleType type, int spriteNum_, glm::vec4 color_, ParticleEffect particE)
{
    position       = position_;
    size           = size_;
    speed          = speed_;
    angle          = angle_;
    this->lifeTime = lifeTime_;
    
    particleType = type;
    spriteNum = spriteNum_;
    color = color_;
    effect = particE;

    switch (particleType)
    {
    case ParticleType::ANIMESPRI:
        sprite = new Sprite();
        sprite->AddMeshWithTexel(spriteNum);
        sprite->SetColor(color);
        break;
    case ParticleType::SPRI:
        sprite = new Sprite();
        sprite->AddMeshWithTexture(spriteNum);
        sprite->SetColor(color);
        break;
    case ParticleType::REC:
        sprite = new Sprite();
        sprite->AddQuad(color);
        break;
    }
}

Particle::~Particle()
{
    //delete sprite;
}

void Particle::Update(float dt)
{
    position.x += speed.x * dt;
    position.y += speed.y * dt;
    //pPhysics.Gravity(dt);
    pPhysics.UpdateForParticle(dt, position);
    switch (effect)
    {
    case ParticleEffect::NORMAL:
        lifeTime -= dt;
        break;
    case ParticleEffect::FADEOUT:
        color.a -= fadeOutAmount * dt;
        lifeTime -= fadeOutAmount * dt;
        break;
    default:
        break;
    }

    sprite->UpdateModel(position, size, angle);
    sprite->UpdateProjection();
    sprite->UpdateView();
    sprite->SetColor(color);
    if (size.x < 0)
    {
        size.x = 0;
    }
    if (size.y < 0)
    {
        size.y = 0;
    }
}
