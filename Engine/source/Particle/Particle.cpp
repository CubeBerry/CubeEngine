//Author: DOYEONG LEE
//Project: CubeEngine
//File: Particle.hpp
#include "Particle/Particle.hpp"
#include "Engine.hpp"

Particle::Particle(glm::vec3 position_, glm::vec3 size_, glm::vec3 speed_, float angle_, float lifeTime_, ParticleType type, std::string spriteName_, glm::vec4 color_, ParticleEffect particE)
{
	position = position_;
	size = size_;
	speed = speed_;
	angle = angle_;
	this->lifeTime = lifeTime_;

	particleType = type;
	spriteName = spriteName_;
	color = color_;
	effect = particE;

	switch (particleType)
	{
	case ParticleType::ANIMESPRI:
		sprite = new Sprite();
		sprite->LoadAnimation(spriteName, spriteName);
		sprite->SetColor(color);
		sprite->PlayAnimation(0);
		break;
	case ParticleType::SPRI:
		sprite = new Sprite();
		sprite->AddMeshWithTexel(spriteName);
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
}

void Particle::Update(float dt)
{
	position.x += speed.x * dt;
	position.y += speed.y * dt;
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


	sprite->UpdateModel({ position.x, position.y, position.z }, size, angle);
	sprite->UpdateProjection();
	sprite->UpdateView();
	sprite->SetColor(color);

	switch (particleType)
	{
	case ParticleType::ANIMESPRI:
		sprite->UpdateAnimation(dt);
		if (sprite->IsAnimationDone() == true)
		{
			std::cout << "Done" << std::endl;
			lifeTime = 0.f;
		}
		break;
	case ParticleType::SPRI:
		break;
	case ParticleType::REC:
		break;
	}

	if (size.x < 0)
	{
		size.x = 0;
	}
	if (size.y < 0)
	{
		size.y = 0;
	}
}
