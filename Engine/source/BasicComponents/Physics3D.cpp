
//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics3D.cpp
#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/Sprite.hpp"

#ifdef _DEBUG
#include "Engine.hpp"
#endif // _DEBUG

#include <iostream>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

Physics3D::~Physics3D()
{
#ifdef _DEBUG
	for (auto& v : points)
	{
		delete v.sprite;
	}
	points.clear();
#endif
}

void Physics3D::Init()
{
#ifdef _DEBUG
#endif // _DEBUG
}

void Physics3D::Update(float dt)
{
	//float fixedTimeStep = 1.0f / 60.f;
	//int steps = static_cast<int>(dt / fixedTimeStep);
	//float remainingTime = dt - (steps * fixedTimeStep);

	//for (int i = 0; i < steps; ++i)
	//{
	//	UpdatePhysics(remainingTime);
		UpdatePhysics(dt);
	//}

	//if (remainingTime > 0.0f)
	//{
	//	UpdatePhysics(remainingTime);
	//}

#ifdef _DEBUG
	std::vector<glm::vec3> rotatedPoints1;
	glm::quat rotation1 = glm::quat(glm::radians(-GetOwner()->GetRotate3D()));

	int i = 0;
	for (auto& v : points)
	{
		if (i == 0)
		{
			i++;
		}
		else
		{
			rotatedPoints1.push_back(RotatePoint(v.pos, GetOwner()->GetPosition(), rotation1));
		}
	}
	for (auto& v : rotatedPoints1)
	{
		v;
		//v;
		//glm::vec3 A = rotatedPoints1[i - 1];
		glm::vec3 B = { 0.f, 0.f, 0.f };
		if (rotatedPoints1.size() > i)
		{
			B = rotatedPoints1[i];
		}
		else
		{
			B = rotatedPoints1[0];
		}

		points[i].sprite->UpdateModel(B, { 0.1f,0.1f,0.1f }, 0.f);
		points[i].sprite->UpdateProjection();
		points[i].sprite->UpdateView();
		i++;
	}
	glm::vec3 centerP = FindSATCenter(rotatedPoints1);
	points[0].sprite->UpdateModel({ centerP.x, centerP.y ,centerP.z }, { 0.1f,0.1f,0.1f }, 0.f);
	points[0].sprite->UpdateProjection();
	points[0].sprite->UpdateView();
#endif // _DEBUG

}

void Physics3D::UpdatePhysics(float dt)
{
	{
	//	acceleration.x = force.x / mass;
	//	velocity.x += acceleration.x * dt;
	//	velocity.x *= friction;

	//	acceleration.y = force.y / mass;
	//	velocity.y += acceleration.y * dt;
	//	if (isGravityOn == false)
	//	{
	//		velocity.y *= friction;
	//	}

	//	acceleration.z = force.z / mass;
	//	velocity.z += acceleration.z * dt;
	//	velocity.z *= friction;

	//	force = { 0.f, 0.f, 0.f };

	//	if (std::abs(velocity.x) < velocityMin.x)
	//	{
	//		velocity.x = 0.f;
	//	}
	//	if (std::abs(velocity.y) < velocityMin.y)
	//	{
	//		velocity.y = 0.f;
	//	}
	//	if (std::abs(velocity.z) < velocityMin.z)
	//	{
	//		velocity.z = 0.f;
	//	}

	//	// 회전 운동 업데이트
	//	/*glm::vec3 angularAcceleration = glm::mat3_cast(orientation) * inverseInertia * glm::mat3_cast(glm::conjugate(orientation)) * torque;
	//	angularVelocity += angularAcceleration * dt;
	//	angularVelocity *= (1.0f - friction);

	//	glm::quat spin(0, angularVelocity.x * dt * 0.5f, angularVelocity.y * dt * 0.5f, angularVelocity.z * dt * 0.5f);
	//	orientation = glm::normalize(orientation + spin * orientation);*/
	//	// 회전 운동 업데이트

	//	Component::GetOwner()->SetXPosition(Component::GetOwner()->GetPosition().x + velocity.x);
	//	Component::GetOwner()->SetYPosition(Component::GetOwner()->GetPosition().y + velocity.y);
	//	Component::GetOwner()->SetZPosition(Component::GetOwner()->GetPosition().z + velocity.z);

	//	//orientation = glm::quat(Component::GetOwner()->GetRotate3D());
	//	//Component::GetOwner()->SetRotate(glm::eulerAngles(orientation));
	}

	acceleration = force / mass;
	Gravity(dt);
	velocity += acceleration * dt /** 60.f*/;
	//float frictionFactor = std::pow(friction, dt /** 60.f*/);
	
	if (!isGravityOn)
	{
		velocity *= friction;
	}
	else
	{
		velocity.x *= friction;
		velocity.z *= friction;
	}

	velocity = glm::vec3(
		std::abs(velocity.x) < velocityMin.x ? 0.f : velocity.x,
		std::abs(velocity.y) < velocityMin.y ? 0.f : velocity.y,
		std::abs(velocity.z) < velocityMin.z ? 0.f : velocity.z
	);

	Component::GetOwner()->SetPosition(Component::GetOwner()->GetPosition() + velocity);
	force = { 0.f, 0.f, 0.f };
}

void Physics3D::UpdateForParticle(float dt, glm::vec3& pos)
{
	acceleration.x = force.x / mass;
	velocity.x += acceleration.x * dt;
	velocity.x *= friction;

	acceleration.y = force.y / mass;
	velocity.y += acceleration.y * dt;
	if (isGravityOn == false)
	{
		velocity.y *= friction;
	}

	acceleration.z = force.z / mass;
	velocity.z += acceleration.z * dt;
	velocity.z *= friction;

	force = { 0.f, 0.f, 0.f };

	if (std::abs(velocity.x) < velocityMin.x)
	{
		velocity.x = 0.f;
	}
	if (std::abs(velocity.y) < velocityMin.y)
	{
		velocity.y = 0.f;
	}
	if (std::abs(velocity.z) < velocityMin.z)
	{
		velocity.z = 0.f;
	}

	pos.x = (pos.x + velocity.x);
	pos.y = (pos.y + velocity.y);
	pos.z = (pos.z + velocity.z);
}

void Physics3D::Gravity(float dt)
{
	if (isGravityOn)
	{
		velocity.y -= gravity * dt /** 60.f*/;
		if (std::abs(velocity.y) > velocityMax.y)
		{
			velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
		}
	}
}

bool Physics3D::CheckCollision(Object* obj)
{
	switch (colliderType)
	{
	case ColliderType3D::BOX:
		if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::BOX)
		{
			return CollisionPP(GetOwner(), obj);
		}
		else if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::SPHERE)
		{
			//return CollisionPC(GetOwner(), obj);
		}
		break;
	case ColliderType3D::SPHERE:
		if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::SPHERE)
		{
			//return CollisionPC(obj, GetOwner());
		}
		else if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::SPHERE)
		{
			//return CollisionCC(GetOwner(), obj);
		}
		break;
	default:
		return false;
		break;
	}
	return false;
}

bool Physics3D::CollisionPP(Object* obj, Object* obj2)
{
	if (obj->GetComponent<Physics3D>()->GetCollidePolyhedron().empty() == false && obj2->GetComponent<Physics3D>()->GetCollidePolyhedron().empty() == false)
	{
		const auto& poly1 = obj->GetComponent<Physics3D>()->collidePolyhedron;
		const auto& poly2 = obj2->GetComponent<Physics3D>()->collidePolyhedron;

		if (poly1.empty() || poly2.empty()) 
		{
			return false;
		}

		std::vector<glm::vec3> rotatedPoly1, rotatedPoly2;
		glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), obj->GetPosition()) * glm::mat4_cast(glm::quat(-glm::radians(obj->GetRotate3D())));
		glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), obj2->GetPosition()) * glm::mat4_cast(glm::quat(-glm::radians(obj2->GetRotate3D())));

		for (const auto& point : poly1)
			rotatedPoly1.push_back(glm::vec3(transform1 * glm::vec4(point, 1.0f)));
		for (const auto& point : poly2)
			rotatedPoly2.push_back(glm::vec3(transform2 * glm::vec4(point, 1.0f)));


		std::vector<glm::vec3> axes;
		axes.push_back(glm::vec3(1, 0, 0));
		axes.push_back(glm::vec3(0, 1, 0));
		axes.push_back(glm::vec3(0, 0, 1));

		for (size_t i = 0; i < rotatedPoly1.size(); ++i) 
		{
			for (size_t j = 0; j < rotatedPoly2.size(); ++j) 
			{
				glm::vec3 edge1 = rotatedPoly1[(i + 1) % rotatedPoly1.size()] - rotatedPoly1[i];
				glm::vec3 edge2 = rotatedPoly2[(j + 1) % rotatedPoly2.size()] - rotatedPoly2[j];
				glm::vec3 axis = glm::cross(edge1, edge2);
				if (glm::length(axis) > 0.0001f) 
				{
					axes.push_back(glm::normalize(axis));
				}
			}
		}

		float minDepth = std::numeric_limits<float>::max();
		glm::vec3 collisionNormal (0.f);

		for (const auto& axis : axes)
		{
			float min1, max1, min2, max2;
			float depth;
			if (IsSeparatingAxis(axis, rotatedPoly1, rotatedPoly2, &depth, &min1, &max1, &min2, &max2))
			{
				return false;
			}
			if (depth < minDepth)
			{
				minDepth = depth;
				collisionNormal = axis;
			}
		}

		glm::vec3 centerDiff = FindSATCenter(rotatedPoly2) - FindSATCenter(rotatedPoly1);
		if (glm::dot(centerDiff, collisionNormal) < 0)
			collisionNormal = -collisionNormal;

		auto* physics1 = obj->GetComponent<Physics3D>();
		auto* physics2 = obj2->GetComponent<Physics3D>();

		if (!physics1->GetIsGhostCollision() && !physics2->GetIsGhostCollision())
		{
			glm::vec3 moveVector = collisionNormal * (minDepth / 2.0f);

			if (physics1->GetBodyType() == BodyType3D::RIGID)
				obj->SetPosition(obj->GetPosition() - moveVector);
			if (physics2->GetBodyType() == BodyType3D::RIGID)
				obj2->SetPosition(obj2->GetPosition() + moveVector);

			CalculateLinearVelocity(*physics1, *physics2, collisionNormal, &minDepth);
		}
		return true; 
	}
	return false;
}

void Physics3D::AddCollidePolyhedron(glm::vec3 position)
{
	colliderType = ColliderType3D::BOX;
	collidePolyhedron.push_back(position);
#ifdef _DEBUG
	AddPoint(position);
#endif
}

void Physics3D::AddCollidePolyhedronAABB(glm::vec3 min, glm::vec3 max)
{
	colliderType = ColliderType3D::BOX;
	collidePolyhedron.clear();
	collidePolyhedron = 
	{
	   {min.x, min.y, min.z}, {min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, min.y, min.z},
	   {min.x, min.y, max.z}, {min.x, max.y, max.z}, {max.x, max.y, max.z}, {max.x, min.y, max.z}
	};
#ifdef _DEBUG
	for (auto& v : points)
	{
		delete v.sprite;
	}
	points.clear();

	for(auto poly : collidePolyhedron)
	{
		AddPoint(poly);
	}
#endif
}

void Physics3D::AddCollidePolyhedronAABB(glm::vec3 size)
{
	AddCollidePolyhedronAABB(-size / 2.f, size / 2.f);
}

glm::vec3 Physics3D::FindSATCenter(const std::vector<glm::vec3>& points_)
{
	glm::vec3 center(0.0f);
	for (const auto& point : points_) 
	{
		center += point;
	}
	return center / static_cast<float>(points_.size());
}

glm::vec3 Physics3D::RotatePoint(const glm::vec3& point, const glm::vec3& position, const glm::quat& rotation) //WIP
{
	return (rotation * point) + position;
}

bool Physics3D::IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> points1, const std::vector<glm::vec3> points2, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_)
{
	float min1 = INFINITY;
	float min2 = INFINITY;
	float max1 = -INFINITY;
	float max2 = -INFINITY;

	for (const glm::vec3& vertex : points1)
	{
		float projection = glm::dot(axis, vertex);
		min1 = std::min(min1, projection);
		max1 = std::max(max1, projection);
	}
	for (const glm::vec3& vertex : points2)
	{
		float projection = glm::dot(axis, vertex);
		min2 = std::min(min2, projection);
		max2 = std::max(max2, projection);
	}

	*axisDepth = std::min(max2 - min1, max1 - min2);
	*min1_ = min1;
	*max1_ = max1;
	*min2_ = min2;
	*max2_ = max2;
	return !(max1 >= min2 && max2 >= min1);
}

void Physics3D::CalculateLinearVelocity(Physics3D& body, Physics3D& body2, glm::vec3 normal, float* /*axisDepth*/)
{
	glm::vec3 relativeVelocity = body2.GetVelocity() - body.GetVelocity();
	float res = std::min(body.GetRestitution(), body2.GetRestitution());
	float j = -(1.f - res) * glm::dot(relativeVelocity, normal);
	j /= (1.f / body.mass) + (1.f / body2.mass);
	glm::vec3 impulse = j * normal;

	if (body.GetBodyType() == BodyType3D::RIGID)
	{
		body.SetVelocity(body.GetVelocity() - impulse / body.mass);
	}
	if (body2.GetBodyType() == BodyType3D::RIGID)
	{
		body2.SetVelocity(body2.GetVelocity() + impulse / body2.mass);
	}
}

#ifdef _DEBUG
void Physics3D::AddPoint(glm::vec3 pos)
{
	
	if (points.empty() == true)
	{
		Point3D temp;
		temp.pos = { 0.f,0.f,0.f };
		temp.sprite = new Sprite();
		temp.sprite->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 1.0, 0.0, 0.0, 1.0 });
		points.push_back(std::move(temp));
	}
	Point3D temp;
	temp.pos = pos;
	temp.sprite = new Sprite();
	switch (bodyType)
	{
	case BodyType3D::RIGID:
		temp.sprite->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 1.0, 0.0, 1.0 });
		break;
	case BodyType3D::BLOCK:
		temp.sprite->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 1.0, 0.0, 0.0, 1.0 });
		break;
	default:
		temp.sprite->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/cube.obj", 1, 1, { 0.0, 1.0, 0.0, 1.0 });
		break;
	}
	points.push_back(std::move(temp));
}
#endif
