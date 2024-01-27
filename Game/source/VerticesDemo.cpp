//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: VerticesDemo.cpp
#include "VerticesDemo.hpp"
#include "Engine.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "BasicComponents/Sprite.hpp"
#include "BasicComponents/Physics2D.hpp"

#include <iostream>

void CollideObjects()
{
	for (auto& target : Engine::GetObjectManager()->GetObjectMap())
	{
		for (auto& object : Engine::GetObjectManager()->GetObjectMap())
		{
			if (target.second != nullptr && object.second != nullptr && target.second != object.second
				&& target.second->HasComponent<Physics2D>() == true && object.second->HasComponent<Physics2D>() == true)
			{
				if (target.second->GetComponent<Physics2D>()->CheckCollision(*object.second) == true)
				{
					//std::cout << "Collide " << target.first << " with " << object.first << std::endl;
				}
			}
		}
	}
}

void VerticesDemo::Init()
{
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample2.jpg");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample.png");
	Engine::Instance().GetVKRenderManager()->LoadTexture("../Game/assets/texture_sample3.jpg");

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 1280.f,720.f,0.f }, "0", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(1);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 512.f,512.f,0.f }, "1", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddMeshWithTexture(2);

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 640.f,360.f,0.f }, glm::vec3{ 256.f, 128.f,0.f }, "2", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuadLine({ 1.f,0.f,1.f,1.f });

	Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 70.f, 120.f,0.f }, "3", ObjectType::NONE);
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->LoadAnimation("../Game/assets/player.spt", 3);

	/*Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ 0.f,0.f,0.f }, glm::vec3{ 32.f, 32.f,0.f }, "A", ObjectType::NONE);
	
	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddSpriteToManager();

	Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({32.f,16.f});
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ 0.f,32.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -32.f,16.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ -16.f,-32.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygon({ 16.f,-32.f });
	Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(BodyType::RIGID);
	
	for (int i = 0; i < 20; i++)
	{
		int fric = (rand() % (2 + 1)) + 1;
		int type1 = rand() % (1 - 0 + 1) - 0;
		int mass = (rand() % (8 + 1)) + 1;
		float x = (float)(rand() % (8 - (-10) + 1) - 8) * 32.f;
		float y = (float)(rand() % (8 - (-10) + 1) - 8) * 32.f;
		Engine::Instance().GetObjectManager()->AddObject<Object>(glm::vec3{ x,y,0.f }, glm::vec3{ 32.f, 32.f,0.f }, std::to_string((i + 2)), ObjectType::NONE);

		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Sprite>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddQuad({ 1.f,1.f,1.f,1.f });
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->AddSpriteToManager();

		Engine::Instance().GetObjectManager()->GetLastObject()->AddComponent<Physics2D>();
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetMass(static_cast<float>(mass));
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetBodyType(static_cast<BodyType>(0));

		if (type1 == 0)
		{
			Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollidePolygonAABB({ 16.f, 16.f });
		}
		else
		{
			Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->AddCollideCircle(16.f);
		}
		switch (fric)
		{
		case 1:
			Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(1.f);
			break;
		case 2:
			Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Physics2D>()->SetFriction(0.9f);
			break;
		}
	}*/
}

void VerticesDemo::Update(float dt)
{
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_1))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<Sprite>()->ChangeTexture(0.f);
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_2))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<Sprite>()->ChangeTexture(1.f);
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::NUMBER_3))
		Engine::Instance().GetObjectManager()->FindObjectWithId(1)->GetComponent<Sprite>()->ChangeTexture(2.f);

	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::DOWN))
	{
		//Engine::Instance().GetCameraManager()->MoveUp(5.f);
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceY(20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::UP))
	{
		//Engine::Instance().GetCameraManager()->MoveUp(-5.f);
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceY(-20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::LEFT))
	{
		//Engine::Instance().GetCameraManager()->MoveRight(-5.f);
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceX(-20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::RIGHT))
	{
		//Engine::Instance().GetCameraManager()->MoveRight(5.f);
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetComponent<Physics2D>()->AddForceX(20.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::C))
	{
		//Engine::Instance().GetCameraManager()->MoveRight(5.f);
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetRotate() - 5.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::V))
	{
		//Engine::Instance().GetCameraManager()->MoveRight(5.f);
		Engine::Instance().GetObjectManager()->FindObjectWithName("A")->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithName("A")->GetRotate() + 5.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::A))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() - 10.f);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::S))
	{
		Engine::Instance().GetCameraManager()->SetRotate(Engine::Instance().GetCameraManager()->GetRotate2D() + 10.f);
	}
	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::Z))
	{
		Engine::Instance().GetCameraManager()->SetZoom(Engine::Instance().GetCameraManager()->GetZoom() - 5.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::X))
	{
		Engine::Instance().GetCameraManager()->SetZoom(Engine::Instance().GetCameraManager()->GetZoom() + 5.f * dt);
	}
	else if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::Q))
	{
		Engine::Instance().GetObjectManager()->Destroy(Engine::Instance().GetObjectManager()->GetLastObjectID() - 2);
	}
	//Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetRotate() + 50.f * dt);
	Engine::Instance().GetObjectManager()->FindObjectWithId(2)->SetRotate(Engine::Instance().GetObjectManager()->FindObjectWithId(2)->GetRotate() + 50.f * dt);

	if (Engine::Instance().GetInputManager()->IsKeyPressed(KEYBOARDKEYS::SPACE))
		Engine::Instance().GetObjectManager()->GetLastObject()->GetComponent<Sprite>()->PlayAnimation(0);

	CollideObjects();
}

#ifdef _DEBUG
void VerticesDemo::ImGuiDraw(float /*dt*/)
{
	ImGui::ShowDemoWindow();
	Engine::GetSoundManager()->MusicPlayerForImGui();
}
#endif

void VerticesDemo::Restart()
{
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

void VerticesDemo::End()
{
	Engine::Instance().GetObjectManager()->DestroyAllObjects();
}

