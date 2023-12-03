#pragma once
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "Component.hpp"

enum class DrawType
{
	NONE,
	RECTANGLE,
	RECTANGLELINE,
	SPRITE,
	SPRITEANIMATION,
};

enum class ObjectType
{
	NONE,
};

class Object
{
public:
	Object() = default;
	Object(glm::vec3 pos_, glm::vec3 size_, std::string name = "", ObjectType objectType = ObjectType::NONE);
	Object(const Object& rhs);
	virtual ~Object() = default;
	virtual void Init();
	virtual void Update(float dt);
	virtual void Draw(float dt);

	int        GetId() { return id; }
	glm::vec3     GetPosition() { return position; }
	glm::vec3     GetSize() { return size; }
	glm::vec3     GetSpeed() { return speed; }

	glm::vec3     GetMin() { return min; }
	glm::vec3     GetMax() { return max; }
	float GetRotate() { return angle; }
	glm::vec4 GetColor() { return color; }

	DrawType   GetDrawType() { return drawType; }
	ObjectType GetObjectType() { return objectType; }
	std::string GetName() { return objectName; }
	std::string GetSpriteName() { return spriteName; }
	
	void       SetId(int value) { id = value; }
	void       SetRotate(float value);

	void       SetXPosition(float newX) { position.x = newX; }
	void       SetYPosition(float newY) { position.y = newY; }
	void       SetZPosition(float newZ) { position.z = newZ; }
	void       SetXSpeed(float newX) { speed.x = newX; }
	void       SetYSpeed(float newY) { speed.y = newY; }
	void       SetZSpeed(float newZ) { speed.z = newZ; }
	void       SetXSize(float newX) { size.x = newX; }
	void       SetYSize(float newY) { size.y = newY; }
	void       SetZSize(float newZ) { size.z = newZ; }

	void       SetName(std::string name) { objectName = name; }
	void       SetSpriteName(std::string name);
	void       SetDrawType(DrawType type) { drawType = type; }
	void       SetObjectType(ObjectType type) { objectType = type; }

	void       SetCollisionBoxPosition(float x, float y, float z) { collisionBoxPosition = glm::vec3{ x, y, z }; }
	void       SetCollisionBoxSize(float x, float y, float z) { collisionBoxSize = glm::vec3{ x, y, z }; }
	void       SetColor(glm::vec4 color4) { color = color4; }

	glm::vec3 GetCollisionBoxSize() { return collisionBoxSize; }

	void  SetMinMax();

	bool isCollideWithObjectIn2D(Object& object) noexcept;
	bool isCollideWithPointIn2D(glm::vec3 point) noexcept;


	template <typename ComponentTypes> bool HasComponent()
	{
		for (auto list : componentList)
		{
			if (typeid(*list).name() == typeid(ComponentTypes).name())
				return true;
		}
		return false;
	}

	template<typename ComponentTypes> constexpr void AddComponent()
	{
		if (HasComponent<ComponentTypes>())
		{
			return;
		}
		ComponentTypes* componentType = new ComponentTypes();
		dynamic_cast<Component*>(componentType)->SetOwner(this);
		this->componentList.push_back(componentType);
	}

	template<typename ComponentTypes> ComponentTypes* GetComponent()
	{
		for (auto list : componentList)
		{
			if (typeid(*list).name() == typeid(ComponentTypes).name())
				return dynamic_cast<ComponentTypes*>(list);
		}
		return nullptr;
	}

protected:
	glm::vec3 position{ 0.f, 0.f, 0.f };
	glm::vec3 speed{ 0.f, 0.f, 0.f };
	glm::vec3 size{ 0.f, 0.f, 0.f };
	float angle = 0.f;

	glm::vec3 min{ 0.f, 0.f, 0.f };
	glm::vec3 max{ 0.f, 0.f, 0.f };
	glm::vec3 collisionBoxSize{ 0.f, 0.f, 0.f };
	glm::vec3 collisionBoxPosition{ 0.f,0.f, 0.f };
	glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f);

	int        id = 0;
	DrawType   drawType = DrawType::NONE;
	ObjectType objectType = ObjectType::NONE;

	std::string objectName = "";
	std::string spriteName = "";
	bool isDrawAble = true;
	std::vector<Component*> componentList;
};