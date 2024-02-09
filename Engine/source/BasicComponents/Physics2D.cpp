//Author: DOYEONG LEE
//Project: CubeEngine
//File: Physics2D.cpp
#include "BasicComponents/Physics2D.hpp"
#include "BasicComponents/Sprite.hpp"

Physics2D::~Physics2D()
{
#ifdef _DEBUG
	for (auto& v : points)
	{
		delete v.sprite;
	}
	points.clear();
#endif
}

void Physics2D::Init()
{
#ifdef _DEBUG
#endif // _DEBUG
}

void Physics2D::Update(float dt)
{
	acceleration.x = force.x / mass;
	acceleration.y = force.y / mass;

	velocity.x += acceleration.x * dt;
	velocity.y += acceleration.y * dt;

	velocity.x *= friction;
	velocity.y *= friction;

	force = { 0.f, 0.f };

	if (std::abs(velocity.x) < velocityMin.x)
	{
		velocity.x = 0.f;
	}
	if (std::abs(velocity.y) < velocityMin.y)
	{
		velocity.y = 0.f;
	}

	Component::GetOwner()->SetXPosition(Component::GetOwner()->GetPosition().x + velocity.x);
	Component::GetOwner()->SetYPosition(Component::GetOwner()->GetPosition().y + velocity.y);

#ifdef _DEBUG
	std::vector<glm::vec2> rotatedPoints1;
	int i = 0;
	for (auto& v : points)
	{
		if (i == 0)
		{
			i++;
		}
		else
		{
			rotatedPoints1.push_back(RotatePoint(Component::GetOwner()->GetPosition(), v.pos, DegreesToRadians(Component::GetOwner()->GetRotate())));
		}
	}
	for (auto& v : rotatedPoints1)
	{
		glm::vec2 A = rotatedPoints1[i - 1];
		glm::vec2 B = { 0.f,0.f };
		if (points.size() > i + 1)
		{
			B = rotatedPoints1[i];
		}
		else
		{
			B = rotatedPoints1[0];
		}
		glm::vec2 midPoint = { (A.x + B.x) / 2.f, (A.y + B.y) / 2.f };
		float angle = std::atan2(B.y - A.y, B.x - A.x);
		float distance = Distance(A, B);

		points[i].sprite->UpdateModel({ midPoint.x, midPoint.y , 0.f }, { distance, 1.f ,0.f }, angle * 180 / 3.14);
		points[i].sprite->UpdateProjection();
		points[i].sprite->UpdateView();
		i++;
	}
	glm::vec2 centerP = FindSATCenter(rotatedPoints1);
	points[0].sprite->UpdateModel({ centerP.x, centerP.y , 0.f }, { 2.f,2.f,0.f }, 0.f);
	points[0].sprite->UpdateProjection();
	points[0].sprite->UpdateView();
#endif // _DEBUG

}

void Physics2D::UpdateForParticle(float dt, glm::vec3& pos)
{
	acceleration.x = force.x / mass * dt;
	acceleration.y = force.y / mass * dt;

	velocity.x += acceleration.x;
	velocity.y += acceleration.y;

	velocity.x *= friction;
	velocity.y *= friction;

	force = { 0.f, 0.f };

	if (std::abs(velocity.x) < velocityMin.x)
	{
		velocity.x = 0.f;
	}
	if (std::abs(velocity.y) < velocityMin.y)
	{
		velocity.y = 0.f;
	}

	pos.x = (pos.x + velocity.x);
	pos.y = (pos.y + velocity.y);
}

void Physics2D::Gravity(float dt)
{
	velocity.y += 1.0f * gravity * dt;
	if (std::abs(velocity.y) > velocityMax.y)
	{
		velocity.y = velocityMax.y * ((velocity.y < 0.f) ? -1.f : 1.f);
	}
}

bool Physics2D::CheckCollision(Object& obj)
{
	switch (collideType)
	{
	case CollideType::POLYGON:
		if (obj.GetComponent<Physics2D>()->GetCollideType() == CollideType::POLYGON)
		{
			return CollisionPP(*GetOwner(), obj);
		}
		else if (obj.GetComponent<Physics2D>()->GetCollideType() == CollideType::CIRCLE)
		{
			return CollisionPC(*GetOwner(), obj);
		}
		break;
	case CollideType::CIRCLE:
		if (obj.GetComponent<Physics2D>()->GetCollideType() == CollideType::POLYGON)
		{
			return CollisionPC(obj, *GetOwner());
		}
		else if (obj.GetComponent<Physics2D>()->GetCollideType() == CollideType::CIRCLE)
		{
			return CollisionCC(*GetOwner(), obj);
		}
		break;
	default:
		return false;
		break;
	}
	return false;
}

bool Physics2D::CollisionPP(Object& obj, Object& obj2)
{
	if (obj.GetComponent<Physics2D>()->GetCollidePolygon().empty() == false && obj2.GetComponent<Physics2D>()->GetCollidePolygon().empty() == false)
	{
		Physics2D* a = obj.GetComponent<Physics2D>();
		Physics2D* b = obj2.GetComponent<Physics2D>();

		std::vector<glm::vec2> rotatedPoints1;
		std::vector<glm::vec2> rotatedPoints2;
		float depth = INFINITY;
		glm::vec2 normal = { 0.f, 0.f };

		float min1 = INFINITY;
		float min2 = INFINITY;
		float max1 = -INFINITY;
		float max2 = -INFINITY;

		// 첫 번째 다각형 회전
		for (const glm::vec2& point : collidePolygon)
		{
			rotatedPoints1.push_back(RotatePoint(obj.GetPosition(), point, DegreesToRadians(obj.GetRotate())));
		}

		// 두 번째 다각형 회전
		for (const glm::vec2& point : b->GetCollidePolygon())
		{
			rotatedPoints2.push_back(RotatePoint(obj2.GetPosition(), point, DegreesToRadians(obj2.GetRotate())));
		}

		for (size_t i = 0; i < rotatedPoints1.size(); ++i)
		{
			float axisDepth = 0.f;
			glm::vec2 edge = rotatedPoints1[(i + 1) % rotatedPoints1.size()] - rotatedPoints1[i];
			glm::vec2 axis = glm::vec2(-edge.y, edge.x); // 수직인 축
			axis = normalize(axis);
			if (IsSeparatingAxis(axis, rotatedPoints1, rotatedPoints2, &axisDepth, &min1, &max1, &min2, &max2))
			{
				return false; // 충돌이 없음
			}
			if (axisDepth < depth)
			{
				depth = axisDepth;
				normal = axis;
			}
		}

		for (size_t i = 0; i < rotatedPoints2.size(); ++i)
		{
			float axisDepth = 0.f;
			glm::vec2 edge = rotatedPoints2[(i + 1) % rotatedPoints2.size()] - rotatedPoints2[i];
			glm::vec2 axis = glm::vec2(-edge.y, edge.x); // 수직인 축
			axis = normalize(axis); 
			if (IsSeparatingAxis(axis, rotatedPoints1, rotatedPoints2, &axisDepth, &min1, &max1, &min2, &max2))
			{
				return false; // 충돌이 없음
			}
			if (axisDepth < depth)
			{
				depth = axisDepth;
				normal = axis;
			}
		}

		glm::vec2 direction = FindSATCenter(rotatedPoints2) - FindSATCenter(rotatedPoints1);
		if (glm::dot(direction, normal) < 0.f)
		{
			normal = -normal;
		}

		if (a->GetBodyType() == BodyType::RIGID &&
			b->GetBodyType() == BodyType::RIGID)
		{
			obj.SetXPosition(obj.GetPosition().x + (-normal * depth / 2.f).x);
			obj.SetYPosition(obj.GetPosition().y + (-normal * depth / 2.f).y);

			obj2.SetXPosition(obj2.GetPosition().x + (normal * depth / 2.f).x);
			obj2.SetYPosition(obj2.GetPosition().y + (normal * depth / 2.f).y);
		}
		else if (a->GetBodyType() == BodyType::RIGID &&
			b->GetBodyType() == BodyType::BLOCK)
		{
			obj.SetXPosition(obj.GetPosition().x + (-normal * depth / 2.f).x);
			obj.SetYPosition(obj.GetPosition().y + (-normal * depth / 2.f).y);
		}
		else if (a->GetBodyType() == BodyType::BLOCK &&
			b->GetBodyType() == BodyType::RIGID)
		{
			obj2.SetXPosition(obj2.GetPosition().x + (normal * depth / 2.f).x);
			obj2.SetYPosition(obj2.GetPosition().y + (normal * depth / 2.f).y);
		}
		CalculateLinearVelocity(*a, *b, normal, &depth);
		return true; // 모든 축에서 겹침이 없음
	}
	return false;
}

bool Physics2D::CollisionCC(Object& obj, Object& obj2)
{
	// 두 원의 중심 사이의 거리 계산
	float distanceX = obj.GetPosition().x - obj2.GetPosition().x;
	float distanceY = obj.GetPosition().y - obj2.GetPosition().y;
	float distance = std::sqrt(distanceX * distanceX + distanceY * distanceY);

	float depth = INFINITY;
	glm::vec2 normal = { 0.f, 0.f };

	Physics2D* a = obj.GetComponent<Physics2D>();
	Physics2D* b = obj2.GetComponent<Physics2D>();

	// 두 원의 반지름의 합과 거리 비교
	if (distance <= a->GetCircleCollideRadius() + b->GetCircleCollideRadius())
	{
		// 두 원의 중심 사이의 단위 벡터 계산
		float unitX = distanceX / distance;
		float unitY = distanceY / distance;

		// 겹친 양 계산
		float overlap = (a->GetCircleCollideRadius() + b->GetCircleCollideRadius() - distance) / 2.0f;

		normal = normalize(obj2.GetPosition() - obj.GetPosition());
		depth = overlap - distance;

		// 각 원의 위치 조정
		if (a->GetBodyType() == BodyType::RIGID &&
			b->GetBodyType() == BodyType::RIGID)
		{
			obj.SetXPosition(obj.GetPosition().x + (overlap * unitX));
			obj.SetYPosition(obj.GetPosition().y + (overlap * unitY));

			obj2.SetXPosition(obj2.GetPosition().x - (overlap * unitX));
			obj2.SetYPosition(obj2.GetPosition().y - (overlap * unitY));
		}
		else if (a->GetBodyType() == BodyType::RIGID &&
			b->GetBodyType() == BodyType::BLOCK)
		{
			obj.SetXPosition(obj.GetPosition().x + (overlap * unitX));
			obj.SetYPosition(obj.GetPosition().y + (overlap * unitY));
		}
		else if (a->GetBodyType() == BodyType::BLOCK &&
			b->GetBodyType() == BodyType::RIGID)
		{
			obj2.SetXPosition(obj2.GetPosition().x - (overlap * unitX));
			obj2.SetYPosition(obj2.GetPosition().y - (overlap * unitY));
		}
		CalculateLinearVelocity(*a, *b, -normal, &depth);
		return true; // 충돌 발생
	}
	return false; // 충돌 없음
}

bool Physics2D::CollisionPC(Object& poly, Object& cir)
{
	Physics2D* p = poly.GetComponent<Physics2D>();
	Physics2D* c = cir.GetComponent<Physics2D>();

	glm::vec2 circleCenter = cir.GetPosition();
	float circleRadius = c->GetCircleCollideRadius();
	std::vector<glm::vec2> rotatedPoints;

	glm::vec2 normal = { 0.f,0.f };
	float depth = INFINITY;

	float min1 = INFINITY;
	float min2 = INFINITY;
	float max1 = -INFINITY;
	float max2 = -INFINITY;


	for (const glm::vec2& point : collidePolygon)
	{
		rotatedPoints.push_back(RotatePoint(poly.GetPosition(), point, DegreesToRadians(poly.GetRotate())));
	}

	for (size_t i = 0; i < rotatedPoints.size(); ++i)
	{
		float axisDepth = 0.f;
		glm::vec2 edge = rotatedPoints[(i + 1) % rotatedPoints.size()] - rotatedPoints[i];
		glm::vec2 axis = glm::vec2(-edge.y, edge.x); // 수직인 축
		axis = normalize(axis);
		if (IsSeparatingAxis(axis, rotatedPoints, circleCenter, circleRadius, &axisDepth, &min1, &max1, &min2, &max2))
		{
			return false; // 충돌이 없음
		}
		if (axisDepth < depth)
		{
			depth = axisDepth;
			normal = axis;
		}
	}

	{
		glm::vec2 closestPoint = FindClosestPointOnSegment(circleCenter, rotatedPoints);
		float axisDepth = 0.f;
		glm::vec2 axis = closestPoint - circleCenter;
		axis = normalize(axis);
		if (IsSeparatingAxis(axis, rotatedPoints, circleCenter, circleRadius, &axisDepth, &min1, &max1, &min2, &max2))
		{
			return false; // 충돌이 없음
		}
		if (axisDepth < depth)
		{
			depth = axisDepth;
			normal = axis;
		}
	}

	glm::vec2 direction = FindSATCenter(rotatedPoints) - circleCenter;
	if (glm::dot(direction, normal) < 0.f)
	{
		normal = -normal;
	}


	if (p->GetBodyType() == BodyType::RIGID &&
		c->GetBodyType() == BodyType::RIGID)
	{
		poly.SetXPosition(poly.GetPosition().x + (normal * depth / 2.f).x);
		poly.SetYPosition(poly.GetPosition().y + (normal * depth / 2.f).y);

		cir.SetXPosition(cir.GetPosition().x + (-normal * depth / 2.f).x);
		cir.SetYPosition(cir.GetPosition().y + (-normal * depth / 2.f).y);
	}
	else if (p->GetBodyType() == BodyType::RIGID &&
		c->GetBodyType() == BodyType::BLOCK)
	{
		poly.SetXPosition(poly.GetPosition().x + (normal * depth / 2.f).x);
		poly.SetYPosition(poly.GetPosition().y + (normal * depth / 2.f).y);
	}
	else if (p->GetBodyType() == BodyType::BLOCK &&
		c->GetBodyType() == BodyType::RIGID)
	{
		cir.SetXPosition(cir.GetPosition().x + (-normal * depth / 2.f).x);
		cir.SetYPosition(cir.GetPosition().y + (-normal * depth / 2.f).y);
	}
	CalculateLinearVelocity(*p, *c, normal, &depth);
	return true;
}

void Physics2D::AddCollideCircle(float r)
{
#ifdef _DEBUG
	float increment = 3.14f * 2.f / 10.f;
	for (int i = 0; i < 10; i++)
	{
		AddPoint({ r * static_cast<float>(cos(i * increment - (3.14f / 2.f))),
					   r * static_cast<float>(sin(i * increment - (3.14f / 2.f))) });

	}
#endif
	collideType = CollideType::CIRCLE;
	collidePolygon.clear();
	circle.radius = r;
}

void Physics2D::AddCollidePolygon(glm::vec2 position)
{
#ifdef _DEBUG
	AddPoint(position);
#endif
	collideType = CollideType::POLYGON;
	collidePolygon.push_back(position);
}

void Physics2D::AddCollidePolygonAABB(glm::vec2 min, glm::vec2 max)
{
#ifdef _DEBUG
	AddPoint({ min.x, min.y });
	AddPoint({ min.x, max.y });
	AddPoint({ max.x, max.y });
	AddPoint({ max.x, min.y });
#endif
	collideType = CollideType::POLYGON;
	collidePolygon.clear();
	collidePolygon = { {min.x, min.y}, {min.x, max.y}, {max.x, max.y}, {max.x, min.y} };
}

void Physics2D::AddCollidePolygonAABB(glm::vec2 size)
{
#ifdef _DEBUG
	AddPoint({ -size.x, -size.y });
	AddPoint({ -size.x, size.y });
	AddPoint({ size.x, size.y });
	AddPoint({ size.x, -size.y });
#endif
	collideType = CollideType::POLYGON;
	collidePolygon.clear();
	collidePolygon = { {-size.x, -size.y}, {-size.x, size.y}, {size.x, size.y}, {size.x, -size.y} };
}

glm::vec2 Physics2D::FindSATCenter(const std::vector<glm::vec2>& points)
{
	float totalArea = 0.0f;
	float centerX = 0.0f;
	float centerY = 0.0f;

	for (size_t i = 0; i < points.size(); ++i)
	{
		glm::vec2 currentPoint = points[i];
		glm::vec2 nextPoint = points[(i + 1) % points.size()];

		float crossProduct = (currentPoint.x * nextPoint.y) - (nextPoint.x * currentPoint.y);

		totalArea += crossProduct;
		centerX += (currentPoint.x + nextPoint.x) * crossProduct;
		centerY += (currentPoint.y + nextPoint.y) * crossProduct;
	}

	centerX *= 1.f / (3.f * totalArea);
	centerY *= 1.f / (3.f * totalArea);

	return glm::vec2(centerX, centerY);
}

glm::vec2 Physics2D::FindClosestPointOnSegment(const glm::vec2& circleCenter, std::vector<glm::vec2>& vertices)
{
	glm::vec2 returnValue = { 0.f,0.f };
	float minDistance = INFINITY;

	for (int i = 0; i < vertices.size(); i++)
	{
		glm::vec2 v = vertices[i];
		float distance = Distance(v, circleCenter);

		if (distance < minDistance)
		{
			minDistance = distance;
			returnValue = v;
		}
	}
	return returnValue;
}

float Physics2D::calculatePolygonRadius(const std::vector<glm::vec2>& vertices)
{   // 다각형의 중심 계산
	float centerX = 0.0f;
	float centerY = 0.0f;
	for (const glm::vec2& vertex : vertices) {
		centerX += vertex.x;
		centerY += vertex.y;
	}
	centerX /= vertices.size();
	centerY /= vertices.size();

	// 가장 먼 거리 찾기
	float maxDistance = 0.0f;
	for (const glm::vec2& vertex : vertices) {
		float distance = std::sqrt((vertex.x - centerX) * (vertex.x - centerX) + (vertex.y - centerY) * (vertex.y - centerY));
		if (distance > maxDistance) {
			maxDistance = distance;
		}
	}
	return maxDistance;
}

float Physics2D::DegreesToRadians(float degrees)
{
	return degrees * 3.14 / 180.f;
}

glm::vec2 Physics2D::RotatePoint(const glm::vec2& point, const glm::vec2& size, float angle)
{
	float x = point.x + (size.x * cos(angle) - size.y * sin(angle));
	float y = point.y + (size.x * sin(angle) + size.y * cos(angle));
	return glm::vec2(x, y);
}

bool Physics2D::IsSeparatingAxis(const glm::vec2& axis, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_)
{
	float min1 = INFINITY;
	float min2 = INFINITY;
	float max1 = -INFINITY;
	float max2 = -INFINITY;

	for (const glm::vec2& point : points1)
	{
		float projection = (point.x * axis.x) + (point.y * axis.y);
		min1 = std::min(min1, projection);
		max1 = std::max(max1, projection);
	}

	for (const glm::vec2& point : points2)
	{
		float projection = (point.x * axis.x) + (point.y * axis.y);
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

bool Physics2D::IsSeparatingAxis(const glm::vec2& axis, const std::vector<glm::vec2>& pointsPoly, const glm::vec2& pointCir, const float radius, float* axisDepth, float* min1_, float* max1_, float* min2_, float* max2_)
{
	float min1 = INFINITY;
	float min2 = INFINITY;
	float max1 = -INFINITY;
	float max2 = -INFINITY;

	for (const glm::vec2& point : pointsPoly)
	{
		float projection = (point.x * axis.x) + (point.y * axis.y);
		min1 = std::min(min1, projection);
		max1 = std::max(max1, projection);
	}

	glm::vec2 direction = normalize(axis);
	glm::vec2 directionAndRadius = direction * radius;

	glm::vec2 p1 = pointCir + directionAndRadius;
	glm::vec2 p2 = pointCir - directionAndRadius;

	min2 = glm::dot(p1, axis);
	max2 = glm::dot(p2, axis);

	if (min2 > max2)
	{
		// swap the min and max values.
		float t = min2;
		min2 = max2;
		max2 = t;
	}

	*axisDepth = std::min(max2 - min1, max1 - min2);
	*min1_ = min1;
	*max1_ = max1;
	*min2_ = min2;
	*max2_ = max2;
	return !(max1 >= min2 && max2 >= min1);
}

void Physics2D::CalculateLinearVelocity(Physics2D& body, Physics2D& body2, glm::vec2 normal, float* /*axisDepth*/)
{
	glm::vec2 relativeVelocity = body2.GetVelocity() - body.GetVelocity();

	float j = -(1.f + 1.f) * glm::dot(relativeVelocity, normal);
	j /= (1.f / body.mass) + (1.f / body2.mass);

	if (body.GetBodyType() == BodyType::RIGID)
	{
		body.SetVelocity(body.GetVelocity() - j / body.mass * normal);
	}
	if (body2.GetBodyType() == BodyType::RIGID)
	{
		body2.SetVelocity(body2.GetVelocity() + j / body2.mass * normal);
	}
}

#ifdef _DEBUG
void Physics2D::AddPoint(glm::vec2 pos)
{
	if (points.empty() == true)
	{
		Point temp;
		temp.pos = { 0.f,0.f };
		temp.sprite = new Sprite();
		temp.sprite->AddQuad({ 1.f,0.f,0.f,1.f });
		points.push_back(std::move(temp));
	}
	Point temp;
	temp.pos = pos;
	temp.sprite = new Sprite();
	switch (bodyType)
	{
	case BodyType::RIGID:
		temp.sprite->AddQuad({ 0.f,1.f,0.f,1.f });
		break;
	case BodyType::BLOCK:
		temp.sprite->AddQuad({ 0.f,0.f,1.f,1.f });
		break;
	default:
		temp.sprite->AddQuad({ 1.f,1.f,1.f,1.f });
		break;
	}
	points.push_back(std::move(temp));
}
#endif
