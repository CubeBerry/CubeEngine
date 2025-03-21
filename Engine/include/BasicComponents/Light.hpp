//Author: DOYEONG LEE
//Project: CubeEngine
//File: Light.hpp
#pragma once
#include "Component.hpp"
#include"../include/Object.hpp"
#include "Material.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

enum class LightType
{
	DIRECTIONAL,
	POINT,
	NONE
};

class Sprite;
class Light : public Component
{
public:
	Light() : Component(ComponentTypes::LIGHT) { Init(); };
	~Light() override;

	void Init() override;
	void AddLight(LightType lightType_, float ambient_ = 0.1f, float specular_ = 0.5f);
	void Update(float dt) override;
	void End() override;

	void SetXPosition(float x);
	void SetYPosition(float y);
	void SetZPosition(float z);
	void SetPosition(glm::vec3 pos_);

	void SetXRotate(float x);
	void SetYRotate(float y);
	void SetZRotate(float z);
	void SetRotate(glm::vec3 rotate_);

	void SetColor(glm::vec4 color_);
	void SetAmbientStrength(float amount);
	void SetSpecularStrength(float amount);

	void SetConstant(float amount);
	void SetLinear(float amount);
	void SetQuadratic(float amount);

	void SetLightId(int index) { lightlId = index; }

	glm::vec3 GetPosition() { return pos; }
	glm::vec4 GetColor() { return color; }
	glm::vec4 GetRotate() { return rotate; }
	float GetAmbientStrength() { return ambient; }
	float GetSpecularStrength() { return specular; }

	float GetConstant() { return constant; }
	float GetLinear() { return linear; }
	float GetQuadratic() { return quadratic; }

	LightType GetLightType() { return lightType; }
	int GetLightId() { return lightlId; }
private:
	glm::vec3 pos = { 0.f,0.f,0.f };
	glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
	glm::vec4 rotate = { 0.f,0.f,0.f,1.f  };
	float ambient = 0.1f;
	float specular = 0.5f;

	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	LightType lightType = LightType::NONE;
	int lightlId = 0;

	Sprite* pointLight = nullptr;

	ThreeDimension::PointLightUniform pLight;
	ThreeDimension::DirectionalLightUniform dLight;
};