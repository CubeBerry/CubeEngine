//Author: DOYEONG LEE
//Project: CubeEngine
//File: Light.hpp
#pragma once
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

class DynamicSprite;

class Light : public IComponent
{
public:
	Light() : IComponent(ComponentTypes::LIGHT) { Init(); };
	~Light() override;

	void Init() override;
	void AddLight(LightType lightType_, float intensity = 1.f, float constant_ = 1.f, float linear_ = 0.7f, float quadratic_ = 1.8f);
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
	void SetIntensity(float amount);
	void SetAmbientStrength(float amount);
	void SetSpecularStrength(float amount);

	void SetConstant(float amount);
	void SetLinear(float amount);
	void SetQuadratic(float amount);

	void SetLightId(int index) { lightlId = index; }

	glm::vec3 GetPosition() const { return pos; }
	glm::vec4 GetColor() const { return color; }
	float GetIntensity() const { return intensity; }
	glm::vec4 GetRotate() const { return rotate; }
	float GetAmbientStrength() const { return ambient; }
	float GetSpecularStrength() const { return specular; }

	float GetConstant() const { return constant; }
	float GetLinear() const { return linear; }
	float GetQuadratic() const { return quadratic; }

	LightType GetLightType() const { return lightType; }
	int GetLightId() const { return lightlId; }
private:
	glm::vec3 pos = { 0.f,0.f,0.f };
	glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
	glm::vec4 rotate = { 0.f,0.f,0.f,1.f  };
	float ambient = 0.1f;
	float specular = 0.5f;

	float constant = 1.0f;
	float linear = 0.7f;
	float quadratic = 1.8f;

	float intensity = 1.f;
	float radius = 1.f;

	LightType lightType = LightType::NONE;
	int lightlId = 0;

	DynamicSprite* pointLight = nullptr;

	ThreeDimension::PointLightUniform pLight;
	ThreeDimension::DirectionalLightUniform dLight;

	void CalculateRadius();
};