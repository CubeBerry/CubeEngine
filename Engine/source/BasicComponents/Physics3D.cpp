//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics3D.cpp

#include "BasicComponents/Physics3D.hpp"
#include "BasicComponents/Sprite.hpp"

#include "Engine.hpp"
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
	Engine::GetLogger().LogDebug(LogCategory::Object, "Component Deleted : Phy3D");
}

void Physics3D::Init()
{
	Engine::GetLogger().LogDebug(LogCategory::Object, "Component Added : Phy3D");
}

void Physics3D::Update(float dt)
{
	UpdatePhysics(dt);
#ifdef _DEBUG
	if (!points.empty())
	{
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
	}
#endif // _DEBUG

}

void Physics3D::UpdatePhysics(float dt)
{
	if (isGravityOn)
	{
		Gravity(dt);
	}

	acceleration = force / mass;
	velocity += acceleration * dt;

	if (friction > 0.f)
	{
		if (isGravityOn)
		{
			velocity.x *= (1.f - friction * dt);
			velocity.z *= (1.f - friction * dt);
		}
		else
		{
			velocity *= (1.f - friction * dt);
		}
	}

	force = { 0.f, 0.f, 0.f };

	if (std::abs(velocity.x) > velocityMax.x)
	{
		velocity.x = velocityMax.x * ((velocity.x < 0.f) ? -1.f : 1.f);
	}
	if (std::abs(velocity.y) > velocityMax.y)
	{
		velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
	}
	if (std::abs(velocity.z) > velocityMax.z)
	{
		velocity.z = velocityMax.z * ((velocity.z < 0.f) ? -1.f : 1.f);
	}

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

	if (collisionMode == CollisionDetectionMode::CONTINUOUS)
	{
		CollisionResult collision = FindClosestCollision(dt);
		if (collision.hasCollided && collision.timeOfImpact <= 1.0f)
		{
			const float skin_width = 0.005f;
			float move_time = collision.timeOfImpact > skin_width ? collision.timeOfImpact - skin_width : 0.0f;
			GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * dt * move_time);

			CalculateLinearVelocity(*this, *collision.otherObject->GetComponent<Physics3D>(), collision.collisionNormal, nullptr);

			float remaining_time = dt - (dt * move_time);
			if (remaining_time > 0.0f)
			{
				GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * remaining_time);
			}
		}
		else
		{
			GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * dt);
		}
	}
	else
	{
		GetOwner()->SetPosition(GetOwner()->GetPosition() + velocity * dt);
	}
}

void Physics3D::UpdateForParticle(float dt, glm::vec3& pos)
{
	if (isGravityOn)
	{
		Gravity(dt);
	}

	acceleration = force / mass;
	velocity += acceleration * dt;

	if (friction > 0.f)
	{
		if (isGravityOn)
		{
			velocity.x *= (1.f - friction * dt);
			velocity.z *= (1.f - friction * dt);
		}
		else
		{
			velocity *= (1.f - friction * dt);
		}
	}

	force = { 0.f, 0.f, 0.f };

	if (std::abs(velocity.x) > velocityMax.x)
	{
		velocity.x = velocityMax.x * ((velocity.x < 0.f) ? -1.f : 1.f);
	}
	if (std::abs(velocity.y) > velocityMax.y)
	{
		velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
	}
	if (std::abs(velocity.z) > velocityMax.z)
	{
		velocity.z = velocityMax.z * ((velocity.z < 0.f) ? -1.f : 1.f);
	}

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

	pos += velocity * dt;
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


void Physics3D::Teleport(glm::vec3 newPosition)
{
	GetOwner()->SetPosition(newPosition);
	velocity = { 0.f, 0.f, 0.f };
}

void Physics3D::SetMass(float m)
{
	if (m > 0.f)
	{
		mass = m;
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
			return CollisionPS(GetOwner(), obj);
		}
		break;
	case ColliderType3D::SPHERE:
		if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::BOX)
		{
			return CollisionPS(obj, GetOwner());
		}
		else if (obj->GetComponent<Physics3D>()->GetColliderType() == ColliderType3D::SPHERE)
		{
			return CollisionSS(GetOwner(), obj);
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
			rotatedPoly1.emplace_back(glm::vec3(transform1 * glm::vec4(point, 1.0f)));
		for (const auto& point : poly2)
			rotatedPoly2.emplace_back(glm::vec3(transform2 * glm::vec4(point, 1.0f)));


		std::vector<glm::vec3> axes;
		axes.emplace_back(glm::vec3(1, 0, 0));
		axes.emplace_back(glm::vec3(0, 1, 0));
		axes.emplace_back(glm::vec3(0, 0, 1));

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
		glm::vec3 collisionNormal(0.f);

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
			const float correction_percent = 0.8f; // 관통의 80%만 한 프레임에 해결
			glm::vec3 moveVector = collisionNormal * (minDepth * correction_percent);

			if (physics1->GetBodyType() == BodyType3D::RIGID)
				obj->SetPosition(obj->GetPosition() - moveVector / 2.0f);
			if (physics2->GetBodyType() == BodyType3D::RIGID)
				obj2->SetPosition(obj2->GetPosition() + moveVector / 2.0f);

			CalculateLinearVelocity(*physics1, *physics2, collisionNormal, &minDepth);
		}
		return true;
	}
	return false;
}

bool Physics3D::CollisionSS(Object* obj, Object* obj2)
{
	glm::vec3 center1 = obj->GetPosition();
	glm::vec3 center2 = obj2->GetPosition();
	float radius1 = obj->GetComponent<Physics3D>()->sphere.radius / 2.f;
	float radius2 = obj2->GetComponent<Physics3D>()->sphere.radius / 2.f;

	float distanceSquared = glm::length2(center2 - center1);
	float radiusSum = radius1 + radius2;

	if (distanceSquared <= radiusSum * radiusSum)
	{
		float distance = std::sqrt(distanceSquared);
		glm::vec3 normal = (center2 - center1) / distance;
		float depth = radiusSum - distance;

		if (obj->GetComponent<Physics3D>()->GetIsGhostCollision() == false &&
			obj2->GetComponent<Physics3D>()->GetIsGhostCollision() == false)
		{
			if (obj->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID &&
				obj2->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID)
			{
				obj->SetPosition(obj->GetPosition() - normal * depth * 0.5f);
				obj2->SetPosition(obj2->GetPosition() + normal * depth * 0.5f);
			}
			else if (obj->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID &&
				obj2->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::BLOCK)
			{
				obj->SetPosition(obj->GetPosition() - normal * depth);
			}
			else if (obj->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::BLOCK &&
				obj2->GetComponent<Physics3D>()->GetBodyType() == BodyType3D::RIGID)
			{
				obj2->SetPosition(obj2->GetPosition() + normal * depth);
			}
			CalculateLinearVelocity(*obj->GetComponent<Physics3D>(), *obj2->GetComponent<Physics3D>(), normal, &depth);
		}
		return true;
	}
	return false;
}

bool Physics3D::CollisionPS(Object* poly, Object* sph)
{
	Physics3D* polyPhysics = poly->GetComponent<Physics3D>();
	Physics3D* sphPhysics = sph->GetComponent<Physics3D>();

	const glm::vec3 polyPosition = poly->GetPosition();
	const glm::quat polyOrientation = glm::quat(glm::radians(-poly->GetRotate3D()));

	const glm::vec3 sphereCenter = sph->GetPosition();
	const float sphereRadius = sphPhysics->sphere.radius / 2.f;

	glm::vec3 minExtent = polyPhysics->GetCollidePolyhedron()[0];
	glm::vec3 maxExtent = polyPhysics->GetCollidePolyhedron()[6];

	glm::vec3 sphereCenterInLocal = glm::inverse(polyOrientation) * (sphereCenter - polyPosition);

	glm::vec3 closestPointInLocal;
	closestPointInLocal.x = std::max(minExtent.x, std::min(sphereCenterInLocal.x, maxExtent.x));
	closestPointInLocal.y = std::max(minExtent.y, std::min(sphereCenterInLocal.y, maxExtent.y));
	closestPointInLocal.z = std::max(minExtent.z, std::min(sphereCenterInLocal.z, maxExtent.z));

	float distanceSquared = glm::length2(closestPointInLocal - sphereCenterInLocal);

	if (distanceSquared <= (sphereRadius * sphereRadius))
	{
		if (!polyPhysics->GetIsGhostCollision() && !sphPhysics->GetIsGhostCollision())
		{
			glm::vec3 collisionNormalInLocal = sphereCenterInLocal - closestPointInLocal;
			if (glm::length2(collisionNormalInLocal) < 0.0001f)
			{
				collisionNormalInLocal = -sphereCenterInLocal;
			}

			float distance = std::sqrt(distanceSquared);
			float depth = sphereRadius - distance;

			glm::vec3 collisionNormal = glm::normalize(polyOrientation * collisionNormalInLocal);
			glm::vec3 moveVector = collisionNormal * depth;

			if (polyPhysics->GetBodyType() == BodyType3D::RIGID)
			{
				poly->SetPosition(poly->GetPosition() - moveVector * 0.5f);
			}
			if (sphPhysics->GetBodyType() == BodyType3D::RIGID)
			{
				sph->SetPosition(sph->GetPosition() + moveVector * 0.5f);
			}

			CalculateLinearVelocity(*polyPhysics, *sphPhysics, collisionNormal, &depth);
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

	for (auto poly : collidePolyhedron)
	{
		AddPoint(poly);
	}
#endif
}

void Physics3D::AddCollidePolyhedronAABB(glm::vec3 size)
{
	AddCollidePolyhedronAABB(-size / 2.f, size / 2.f);
}

void Physics3D::AddCollideSphere(float r)
{
#ifdef _DEBUG
	for (auto& v : points)
	{
		delete v.sprite;
	}
	points.clear();

	if (points.empty() == true)
	{
		Point3D temp;
		temp.pos = { 0.f,0.f,0.f };
		points.push_back(std::move(temp));
		points.at(points.size() - 1).sprite = new Sprite();
		points.at(points.size() - 1).sprite->AddMesh3D(MeshType::OBJ, "../Game/assets/Models/sphere.obj", 30, 30, { 0.0, 1.0, 0.0, 1.0 });
		//temp.sprite = nullptr;
	}
#endif
	colliderType = ColliderType3D::SPHERE;
	collidePolyhedron.clear();
	sphere.radius = r;
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

	glm::vec3 normalizedAxis = glm::normalize(axis);
	float xAxisInfluence = std::abs(glm::dot(normalizedAxis, glm::vec3(1, 0, 0)));
	float yAxisInfluence = std::abs(glm::dot(normalizedAxis, glm::vec3(0, 1, 0)));
	float zAxisInfluence = std::abs(glm::dot(normalizedAxis, glm::vec3(0, 0, 1)));

	float depthScale = 1.0f +
		(xAxisInfluence * 0.5f) +
		(yAxisInfluence * 0.5f) +
		(zAxisInfluence * 0.5f);

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

	*axisDepth = std::min(max2 - min1, max1 - min2) * depthScale;

	*min1_ = min1;
	*max1_ = max1;
	*min2_ = min2;
	*max2_ = max2;

	// 겹침 검사 
	return !(max1 >= min2 && max2 >= min1);
}

bool Physics3D::IsSeparatingAxis(const glm::vec3 axis, const std::vector<glm::vec3> pointsPoly, const glm::vec3 pointSphere, const float radius, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_)
{
	float min1 = INFINITY;
	float max1 = -INFINITY;
	float min2 = INFINITY;
	float max2 = -INFINITY;

	glm::vec3 normalizedAxis = glm::normalize(axis);
	float xAxisInfluence = std::abs(glm::dot(normalizedAxis, glm::vec3(1, 0, 0)));
	float yAxisInfluence = std::abs(glm::dot(normalizedAxis, glm::vec3(0, 1, 0)));
	float zAxisInfluence = std::abs(glm::dot(normalizedAxis, glm::vec3(0, 0, 1)));

	float depthScale = 1.0f +
		(xAxisInfluence * 0.5f) +
		(yAxisInfluence * 0.5f) +
		(zAxisInfluence * 0.5f);

	for (const glm::vec3& point : pointsPoly)
	{
		float projection = glm::dot(point, axis);
		min1 = std::min(min1, projection);
		max1 = std::max(max1, projection);
	}

	float sphereProjection = glm::dot(pointSphere, axis);
	min2 = sphereProjection - radius;
	max2 = sphereProjection + radius;

	*axisDepth = std::min(max2 - min1, max1 - min2) * depthScale;
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

glm::vec3 Physics3D::FindClosestPointOnSegment(const glm::vec3& sphereCenter, std::vector<glm::vec3>& vertices)
{
	glm::vec3 closestPoint = vertices[0];
	float minDistanceSquared = glm::length2(sphereCenter - closestPoint);

	for (size_t i = 1; i < vertices.size(); ++i)
	{
		const glm::vec3& v = vertices[i];
		float distanceSquared = glm::length2(v - sphereCenter);

		if (distanceSquared < minDistanceSquared)
		{
			minDistanceSquared = distanceSquared;
			closestPoint = v;
		}
	}
	return closestPoint;
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

void Physics3D::ProjectPolygon(const std::vector<glm::vec3>& vertices, const glm::vec3& axis, float& min, float& max)
{
	min = glm::dot(vertices[0], axis);
	max = min;
	for (size_t i = 1; i < vertices.size(); ++i)
	{
		float p = glm::dot(vertices[i], axis);
		if (p < min)
		{
			min = p;
		}
		else if (p > max)
		{
			max = p;
		}
	}
}

bool Physics3D::SweptSATOBB(Physics3D* body1, Physics3D* body2, float dt, CollisionResult& outResult)
{
	Object* obj1 = body1->GetOwner();
	Object* obj2 = body2->GetOwner();

	const auto& poly1 = body1->GetCollidePolyhedron();
	const auto& poly2 = body2->GetCollidePolyhedron();

	if (poly1.empty() || poly2.empty()) 
	{
		return false;
	}

	std::vector<glm::vec3> rotatedPoly1, rotatedPoly2;

	// Build transformation matrix for the first object (body1).
	glm::vec3 eulerAngles1 = glm::radians(obj1->GetRotate3D());
	glm::mat4 rotX1 = glm::rotate(glm::mat4(1.0f), eulerAngles1.x, glm::vec3(1, 0, 0));
	glm::mat4 rotY1 = glm::rotate(glm::mat4(1.0f), eulerAngles1.y, glm::vec3(0, 1, 0));
	glm::mat4 rotZ1 = glm::rotate(glm::mat4(1.0f), eulerAngles1.z, glm::vec3(0, 0, 1));
	glm::mat4 rotationMatrix1 = rotZ1 * rotY1 * rotX1;
	glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), obj1->GetPosition()) * rotationMatrix1;

	// Build transformation matrix for the second object (body2).
	glm::vec3 eulerAngles2 = glm::radians(obj2->GetRotate3D());
	glm::mat4 rotX2 = glm::rotate(glm::mat4(1.0f), eulerAngles2.x, glm::vec3(1, 0, 0));
	glm::mat4 rotY2 = glm::rotate(glm::mat4(1.0f), eulerAngles2.y, glm::vec3(0, 1, 0));
	glm::mat4 rotZ2 = glm::rotate(glm::mat4(1.0f), eulerAngles2.z, glm::vec3(0, 0, 1));
	glm::mat4 rotationMatrix2 = rotZ2 * rotY2 * rotX2;
	glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), obj2->GetPosition()) * rotationMatrix2;

	// Transform polyhedron vertices to world space.
	for (const auto& point : poly1) 
	{
		rotatedPoly1.emplace_back(glm::vec3(transform1 * glm::vec4(point, 1.0f)));
	}
	for (const auto& point : poly2) 
	{
		rotatedPoly2.emplace_back(glm::vec3(transform2 * glm::vec4(point, 1.0f)));
	}

	// Check for initial intersection before performing swept test.
	glm::vec3 initialNormal;
	float initialDepth;
	if (StaticSATIntersection(body1, body2, rotatedPoly1, rotatedPoly2, rotationMatrix1, rotationMatrix2, initialNormal, initialDepth))
	{
		outResult.hasCollided = true;
		outResult.timeOfImpact = 0.0f;
		outResult.otherObject = body2->GetOwner();
		outResult.collisionNormal = initialNormal;
		return true;
	}

	// Calculate relative velocity for the current frame.
	glm::vec3 relativeVelocity = (body2->GetVelocity() - body1->GetVelocity()) * dt;

	// Collect all potential separating axes.
	std::vector<glm::vec3> axes;
	const std::vector<std::vector<int>> faces = 
	{
		{4, 5, 6, 7}, {3, 2, 1, 0}, {7, 6, 2, 3},
		{4, 0, 1, 5}, {1, 2, 6, 5}, {4, 7, 3, 0}
	};
	const std::vector<std::vector<int>> edges = 
	{
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
		{6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	// Axes from face normals of both polyhedrons.
	for (const auto& face : faces) 
	{
		axes.push_back(glm::normalize(glm::cross(rotatedPoly1[face[1]] - rotatedPoly1[face[0]], rotatedPoly1[face[3]] - rotatedPoly1[face[0]])));
	}
	for (const auto& face : faces) 
	{
		axes.push_back(glm::normalize(glm::cross(rotatedPoly2[face[1]] - rotatedPoly2[face[0]], rotatedPoly2[face[3]] - rotatedPoly2[face[0]])));
	}

	// Axes from cross products of edges from both polyhedrons.
	std::vector<glm::vec3> edges1, edges2;
	for (const auto& edge : edges) 
	{
		edges1.push_back(rotatedPoly1[edge[1]] - rotatedPoly1[edge[0]]);
	}
	for (const auto& edge : edges) 
	{
		edges2.push_back(rotatedPoly2[edge[1]] - rotatedPoly2[edge[0]]);
	}
	for (const auto& edge1 : edges1) 
	{
		for (const auto& edge2 : edges2) 
		{
			glm::vec3 axis = glm::cross(edge1, edge2);
			if (glm::length2(axis) > 0.0001f) axes.push_back(glm::normalize(axis));
		}
	}

	float tFirst = 0.0f;
	float tLast = 1.0f;
	glm::vec3 bestNormal;

	for (const auto& axis : axes)
	{
		glm::vec3 normalizedAxis = glm::normalize(axis);
		if (glm::length2(normalizedAxis) < 0.0001f) continue;

		float min1, max1, min2, max2;
		ProjectPolygon(rotatedPoly1, normalizedAxis, min1, max1);
		ProjectPolygon(rotatedPoly2, normalizedAxis, min2, max2);

		float projectedVelocity = glm::dot(relativeVelocity, normalizedAxis);

		// Find the time of first and last contact on this axis.
		if (max1 < min2) {
			if (projectedVelocity <= 0) { // Moving apart or parallel.
				return false;
			}
			float tEnter = (min2 - max1) / projectedVelocity;
			if (tEnter > tFirst) {
				tFirst = tEnter;
				bestNormal = -normalizedAxis;
			}
		}
		else if (max2 < min1) 
		{
			if (projectedVelocity >= 0) 
			{ // Moving apart or parallel.
				return false;
			}
			float tEnter = (max2 - min1) / projectedVelocity;
			if (tEnter > tFirst) 
			{
				tFirst = tEnter;
				bestNormal = normalizedAxis;
			}
		}

		if (projectedVelocity > 0) 
		{
			tLast = std::min(tLast, (max2 - min1) / projectedVelocity);
		}
		else if (projectedVelocity < 0) 
		{
			tLast = std::min(tLast, (min2 - max1) / projectedVelocity);
		}

		// If the intervals do not overlap, a separating axis is found for the whole movement.
		if (tFirst > tLast) 
		{
			return false;
		}
	}

	// If a collision is predicted within the frame time.
	if (tFirst >= 0.0f && tFirst <= 1.0f)
	{
		// Correct the normal direction.
		glm::vec3 center1 = FindSATCenter(rotatedPoly1);
		glm::vec3 center2 = FindSATCenter(rotatedPoly2);
		if (glm::dot(center2 - center1, bestNormal) < 0.0f)
		{
			bestNormal = -bestNormal;
		}

		outResult.hasCollided = true;
		outResult.timeOfImpact = tFirst;
		outResult.otherObject = obj2;
		outResult.collisionNormal = glm::normalize(bestNormal);
		return true;
	}

	return false;
}

bool Physics3D::StaticSATIntersection(
	Physics3D* body1, Physics3D* body2,
	const std::vector<glm::vec3>& rotatedPoly1, const std::vector<glm::vec3>& rotatedPoly2,
	const glm::mat4& rotationMatrix1, const glm::mat4& rotationMatrix2,
	glm::vec3& outNormal, float& outDepth)
{
	Object* obj1 = body1->GetOwner();
	Object* obj2 = body2->GetOwner();

	std::vector<glm::vec3> axes;
	// Get the world-space axes of each OBB (face normals).
	glm::vec3 obb1_axes[3] = 
	{
		glm::normalize(glm::vec3(rotationMatrix1[0])),
		glm::normalize(glm::vec3(rotationMatrix1[1])),
		glm::normalize(glm::vec3(rotationMatrix1[2]))
	};
	glm::vec3 obb2_axes[3] = 
	{
		glm::normalize(glm::vec3(rotationMatrix2[0])),
		glm::normalize(glm::vec3(rotationMatrix2[1])),
		glm::normalize(glm::vec3(rotationMatrix2[2]))
	};

	// 1. Add the 3 face normal axes of the first OBB.
	axes.push_back(obb1_axes[0]);
	axes.push_back(obb1_axes[1]);
	axes.push_back(obb1_axes[2]);

	// 2. Add the 3 face normal axes of the second OBB.
	axes.push_back(obb2_axes[0]);
	axes.push_back(obb2_axes[1]);
	axes.push_back(obb2_axes[2]);

	// 3. Add up to 9 cross-product axes from the edges of both OBBs.
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			glm::vec3 cross_axis = glm::cross(obb1_axes[i], obb2_axes[j]);
			// Avoid adding a zero vector if axes are parallel.
			if (glm::length2(cross_axis) > 0.0001f)
			{
				axes.push_back(glm::normalize(cross_axis));
			}
		}
	}

	float minDepth = std::numeric_limits<float>::max();
	glm::vec3 collisionNormal;

	for (const auto& axis : axes)
	{
		float min1, max1, min2, max2;
		ProjectPolygon(rotatedPoly1, axis, min1, max1);
		ProjectPolygon(rotatedPoly2, axis, min2, max2);

		// Found a separating axis, no collision.
		if (max1 < min2 || max2 < min1)
		{
			return false;
		}

		float depth = std::min(max2 - min1, max1 - min2);
		if (depth < minDepth)
		{
			minDepth = depth;
			collisionNormal = axis;
		}
	}

	// Correct the direction of the collision normal.
	glm::vec3 center1 = FindSATCenter(rotatedPoly1);
	glm::vec3 center2 = FindSATCenter(rotatedPoly2);
	if (glm::dot(center2 - center1, collisionNormal) < 0.0f)
	{
		collisionNormal = -collisionNormal;
	}

	outNormal = glm::normalize(collisionNormal);
	outDepth = minDepth;
	return true; // Overlap on all axes, collision detected.
}

bool Physics3D::SweptSpheres(Physics3D* body1, Physics3D* body2, float dt, CollisionResult& outResult)
{
	Object* obj1 = body1->GetOwner();
	Object* obj2 = body2->GetOwner();

	glm::vec3 pos1 = obj1->GetPosition();
	glm::vec3 pos2 = obj2->GetPosition();
	float r1 = body1->sphere.radius / 2.0f;
	float r2 = body2->sphere.radius / 2.0f;

	// Calculate the relative velocity vector for the current frame.
	glm::vec3 rel_vel_per_second = body2->GetVelocity() - body1->GetVelocity();
	glm::vec3 rel_vel = rel_vel_per_second * dt;

	glm::vec3 start_diff = pos2 - pos1;
	float combined_radius = r1 + r2;

	// Coefficients for the quadratic equation: At^2 + Bt + C = 0
	float a = glm::dot(rel_vel, rel_vel);
	float b = 2.0f * glm::dot(start_diff, rel_vel);
	float c = glm::dot(start_diff, start_diff) - combined_radius * combined_radius;

	// Check for initial overlap. If c < 0, the spheres are already overlapping.
	if (c < 0.0f) 
	{
		outResult.hasCollided = true;
		outResult.timeOfImpact = 0.0f;
		outResult.otherObject = obj2;
		outResult.collisionNormal = glm::normalize(pos1 - pos2);
		return true;
	}

	// Handle the case where there's no relative movement.
	// If not moving and not overlapping, they will never collide.
	if (a < 0.00001f) 
	{
		return false;
	}

	// Use the discriminant to check if a future collision will occur.
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) 
	{
		return false; // No real roots, no collision.
	}

	// Calculate and validate the time of impact (t).
	// Use the smaller root for the first time of impact.
	float t = (-b - sqrt(discriminant)) / (2.0f * a);

	// Check if the collision occurs within this frame.
	if (t >= 0.0f && t <= 1.0f) 
	{
		outResult.hasCollided = true;
		outResult.timeOfImpact = t;
		outResult.otherObject = obj2;
		// Calculate the exact normal at the moment of impact.
		outResult.collisionNormal = glm::normalize((pos1 + body1->GetVelocity() * dt * t) - (pos2 + body2->GetVelocity() * dt * t));
		return true;
	}

	return false;
}

bool Physics3D::SweptSphereVsOBB(Physics3D* boxBody, float dt, CollisionResult& outResult)
{
	Object* sphereObj = this->GetOwner();
	Object* boxObj = boxBody->GetOwner();

	// Create the inverse transform matrix to move into the box's local space.
	glm::mat4 boxRotationMatrix;
	{
		glm::vec3 eulerAngles = glm::radians(boxObj->GetRotate3D());
		boxRotationMatrix = glm::rotate(glm::mat4(1.0f), -eulerAngles.z, glm::vec3(0, 0, 1));
		boxRotationMatrix = glm::rotate(boxRotationMatrix, -eulerAngles.y, glm::vec3(0, 1, 0));
		boxRotationMatrix = glm::rotate(boxRotationMatrix, -eulerAngles.x, glm::vec3(1, 0, 0));
	}
	glm::mat4 boxTransform = glm::translate(glm::mat4(1.0f), boxObj->GetPosition()) * boxRotationMatrix;
	glm::mat4 inverseBoxTransform = glm::inverse(boxTransform);

	// Transform sphere's position and velocity into the box's local space.
	glm::vec3 spherePos_local = glm::vec3(inverseBoxTransform * glm::vec4(sphereObj->GetPosition(), 1.0f));
	glm::vec3 sphereVel_local = glm::vec3(inverseBoxTransform * glm::vec4(this->GetVelocity() * dt, 0.0f));

	glm::vec3 boxHalfExtents = boxBody->GetCollidePolyhedron()[6];
	float sphereRadius = this->sphere.radius / 2.0f;

	float tFirst = 0.0f;
	float tLast = 1.0f;
	glm::vec3 hitNormal_local(0.0f);

	// Test against each of the three slabs of the OBB.
	for (int i = 0; i < 3; ++i)
	{
		float slab_min = -boxHalfExtents[i] - sphereRadius;
		float slab_max = boxHalfExtents[i] + sphereRadius;

		// Check for no collision if velocity is parallel to the slab.
		if (std::abs(sphereVel_local[i]) < 0.00001f)
		{
			if (spherePos_local[i] < slab_min || spherePos_local[i] > slab_max) return false;
		}
		else
		{
			// Calculate time of entry and exit from the slab.
			float tEnter = (slab_min - spherePos_local[i]) / sphereVel_local[i];
			float tLeave = (slab_max - spherePos_local[i]) / sphereVel_local[i];
			if (tEnter > tLeave) 
			{
				std::swap(tEnter, tLeave);
			}

			// Update the overall time of first contact and collision normal.
			if (tEnter >= tFirst)
			{
				tFirst = tEnter;
				hitNormal_local = glm::vec3(0.0f);
				hitNormal_local[i] = (sphereVel_local[i] > 0) ? -1.0f : 1.0f;
			}
			tLast = std::min(tLast, tLeave);

			if (tFirst > tLast) 
			{
				return false; // No overlap in collision intervals.
			}
		}
	}

	// If the time of impact is 0 (initial overlap), calculate a stable push-out normal.
	if (tFirst == 0.0f)
	{
		// Determine the normal based on the vector from the box center to the sphere's local position.
		glm::vec3 closestPointOnBox = glm::clamp(spherePos_local, -boxHalfExtents, boxHalfExtents);
		glm::vec3 direction = spherePos_local - closestPointOnBox;
		if (glm::length2(direction) < 0.0001f) 
		{ 
			// If the sphere's center is inside the box,
			hitNormal_local = -glm::normalize(spherePos_local); // push it out from the center.
		}
		else 
		{
			hitNormal_local = glm::normalize(direction);
		}
	}

	// If a collision occurs within the frame.
	if (tFirst >= 0.0f && tFirst <= 1.0f)
	{
		// Transform the normal to world space using only the rotation part of the matrix.
		glm::mat4 pureRotation = glm::mat4(1.0f);
		glm::vec3 boxEuler = glm::radians(boxObj->GetRotate3D());
		pureRotation = glm::rotate(pureRotation, -boxEuler.z, glm::vec3(0, 0, 1));
		pureRotation = glm::rotate(pureRotation, -boxEuler.y, glm::vec3(0, 1, 0));
		pureRotation = glm::rotate(pureRotation, -boxEuler.x, glm::vec3(1, 0, 0));
		glm::vec3 hitNormal_world = glm::normalize(glm::vec3(glm::inverse(pureRotation) * glm::vec4(hitNormal_local, 0.0f)));

		outResult.hasCollided = true;
		outResult.timeOfImpact = tFirst;
		outResult.otherObject = boxObj;
		outResult.collisionNormal = hitNormal_world;
		return true;
	}

	return false;
}

CollisionResult Physics3D::FindClosestCollision(float dt)
{
	CollisionResult closestCollision;
	closestCollision.timeOfImpact = 1.1f; // Initialize with a time greater than 1.0
	Object* self = GetOwner();
	const auto& allObjects = Engine::GetObjectManager().GetObjectMap();

	for (const auto& pair : allObjects)
	{
		Object* other = pair.second.get();
		if (self == other || !other->HasComponent<Physics3D>()) continue;

		Physics3D* otherBody = other->GetComponent<Physics3D>();
		CollisionResult currentCollision;

		ColliderType3D type1 = this->GetColliderType();
		ColliderType3D type2 = otherBody->GetColliderType();

		// Call the appropriate CCD function based on collider types.
		if (type1 == ColliderType3D::SPHERE && type2 == ColliderType3D::SPHERE)
		{
			// Sphere vs Sphere test.
			if (SweptSpheres(this, otherBody, dt, currentCollision))
			{
				if (currentCollision.timeOfImpact < closestCollision.timeOfImpact)
				{
					closestCollision = currentCollision;
				}
			}
		}
		else if (type1 == ColliderType3D::BOX && type2 == ColliderType3D::BOX)
		{
			// Box vs Box test.
			if (SweptSATOBB(this, otherBody, dt, currentCollision))
			{
				if (currentCollision.timeOfImpact < closestCollision.timeOfImpact)
				{
					closestCollision = currentCollision;
				}
			}
		}
		else if (type1 == ColliderType3D::SPHERE && type2 == ColliderType3D::BOX)
		{
			// Sphere vs Box test.
			if (this->SweptSphereVsOBB(otherBody, dt, currentCollision))
			{
				if (currentCollision.timeOfImpact < closestCollision.timeOfImpact)
				{
					closestCollision = currentCollision;
				}
			}
		}
		else if (type1 == ColliderType3D::BOX && type2 == ColliderType3D::SPHERE)
		{
			// Box vs Sphere test (reusing the function and reversing the normal).
			if (otherBody->SweptSphereVsOBB(this, dt, currentCollision))
			{
				currentCollision.collisionNormal *= -1.0f;
				if (currentCollision.timeOfImpact < closestCollision.timeOfImpact)
				{
					closestCollision = currentCollision;
				}
			}
		}
	}
	return closestCollision;
}