//Author: DOYEONG LEE
//Project: CubeEngine
//File: Light.cpp
#include "BasicComponents/Light.hpp"
#include "BasicComponents/Sprite.hpp"
#include "Engine.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

Light::~Light()
{
	if (lightPoint != nullptr)
	{
		delete lightPoint;
	}
}

void Light::Init()
{
}

void Light::AddLight(LightType lightType_, float ambient_, float specular_)
{
	if (lightType == LightType::None) 
	{
		lightType = lightType_;
		pos = GetOwner()->GetPosition();
		if (lightType == LightType::Direct)
		{
			dLight.lightDirection = { 0.f,0.f,0.f };
			dLight.ambientStrength = ambient_;
			dLight.specularStrength = specular_;
			Engine::GetRenderManager()->AddDirectionalLight(dLight);
			lightlId = static_cast<int>(Engine::GetRenderManager()->GetDirectionalLightUniforms().size() - 1);
		}
		else if (lightType == LightType::Point)
		{
			pLight.lightPosition.x = pos.x;
			pLight.lightPosition.y = pos.y;
			pLight.lightPosition.z = pos.z;

			pLight.ambientStrength = ambient_;
			pLight.specularStrength = specular_;
			Engine::GetRenderManager()->AddPointLight(pLight);
			lightlId = static_cast<int>(Engine::GetRenderManager()->GetPointLightUniforms().size() - 1);

			lightPoint = new Sprite();
			lightPoint->AddMesh3D(MeshType::CUBE, "", 1, 1, color);
		}
	}
}

void Light::Update(float /*dt*/)
{
	if(lightPoint != nullptr)
	{
		lightPoint->UpdateModel(pos, { 0.1f,0.1f,0.1f }, { rotate.x, rotate.y, rotate.z });
		lightPoint->UpdateProjection();
		lightPoint->UpdateView();
		//lightPoint->SetColor(color);
	

		if (lightType == LightType::Direct) 
		{
			Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId] = dLight;
		}
		else if (lightType == LightType::Point) 
		{
			Engine::GetRenderManager()->GetPointLightUniforms()[lightlId] = pLight;
		}
	}
}

void Light::End()
{
}

void Light::SetXPosition(float x)
{
	pos.x = x;
	if (lightType == LightType::Point) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.x = pos.x;
		pLight.lightPosition.x = pos.x;
	}
}

void Light::SetYPosition(float y)
{
	pos.y = y;
	if (lightType == LightType::Point)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.y = pos.y;
		pLight.lightPosition.y = pos.y;
	}
}

void Light::SetZPosition(float z)
{
	pos.z = z;
	if (lightType == LightType::Point)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.z = pos.z;
		pLight.lightPosition.z = pos.z;
	}
}

void Light::SetPosition(glm::vec3 pos_)
{
	pos = pos_;
	if (lightType == LightType::Point)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.x = pos.x;
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.y = pos.y;
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.z = pos.z;


		pLight.lightPosition = pos;
	}
}

void Light::SetXRotate(float x)
{
	//rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-angle.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
	//	glm::rotate(glm::mat4(1.0f), glm::radians(-angle.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
	//	glm::rotate(glm::mat4(1.0f), glm::radians(-angle.z), glm::vec3(0.0f, 0.0f, 1.0f));
	//pos = glm::vec3(pos_.x * 2, pos_.y * 2, pos_.z);
	//modelMatrix = glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *

	if (x > 360.f)
	{
		rotate.x = x - 360.f;

	}
	else if (x < 0.f)
	{
		rotate.x = 360.f + x;
	}
	else
	{
		rotate.x = x;
	}

	if (lightType == LightType::Direct)
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightDirection.x = glm::radians(-rotate.x);
		dLight.lightDirection.x = glm::radians(-rotate.x);
	}
	else if (lightType == LightType::Point) 
	{
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(-rotate.x), glm::vec3(1.f, 0.f, 0.f));
		glm::vec4 rotatedPosition = rotationMatrix * glm::vec4(0.f, 0.f, 1.f, 1.f);
		
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition 
			= glm::vec3{ rotatedPosition.x, rotatedPosition.y, rotatedPosition.z};
		pos.x = rotatedPosition.x;
		pos.y = rotatedPosition.y;
		pos.y = rotatedPosition.z;

		pLight.lightPosition = pos;
	}
}

void Light::SetYRotate(float y)
{
	if (y > 360.f)
	{
		rotate.y = y - 360.f;

	}
	else if (y < 0.f)
	{
		rotate.y = 360.f + y;
	}
	else
	{
		rotate.y = y;
	}

	if (lightType == LightType::Direct) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightDirection.y = glm::radians(-rotate.y);
		dLight.lightDirection.y = glm::radians(-rotate.y);
	}
	else if (lightType == LightType::Point) 
	{
		glm::mat4 rotationMatriy = glm::rotate(glm::mat4(1), glm::radians(-rotate.y), glm::vec3(0.f, 1.f, 0.f));
		glm::vec4 rotatedPosition = rotationMatriy * glm::vec4(0.f, 0.f, 1.f, 1.f);

		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition
			= glm::vec3{ rotatedPosition.x, rotatedPosition.y, rotatedPosition.z };
		pos.x = rotatedPosition.x;
		pos.y = rotatedPosition.y;
		pos.y = rotatedPosition.z;

		pLight.lightPosition = pos;
	}
}

void Light::SetZRotate(float z)
{
	if (z > 360.f)
	{
		rotate.z = z - 360.f;

	}
	else if (z < 0.f)
	{
		rotate.z = 360.f + z;
	}
	else
	{
		rotate.z = z;
	}

	if (lightType == LightType::Direct)
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightDirection.z = glm::radians(-rotate.z);
		dLight.lightDirection.z = glm::radians(-rotate.z);
	}
	else if (lightType == LightType::Point) 
	{
		glm::mat4 rotationMatriz = glm::rotate(glm::mat4(1), glm::radians(-rotate.z), glm::vec3(0.f, 0.f, 1.f));
		glm::vec4 rotatedPosition = rotationMatriz * glm::vec4(0.f, 0.f, 1.f, 1.f);

		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition
			= glm::vec3{ rotatedPosition.x, rotatedPosition.y, rotatedPosition.z };
		pos.x = rotatedPosition.x;
		pos.y = rotatedPosition.y;
		pos.y = rotatedPosition.z;

		pLight.lightPosition = pos;
	}
}

void Light::SetRotate(glm::vec3 rotate_)
{
	SetXRotate(rotate_.x);
	SetYRotate(rotate_.y);
	SetZRotate(rotate_.z);

	if (lightType == LightType::Point) 
	{
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-rotate.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(-rotate.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(-rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec4 rotatedPosition = rotationMatrix * glm::vec4(1.f, 1.f, 1.f, 1.f);

		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition
			= glm::vec3{ rotatedPosition.x, rotatedPosition.y, rotatedPosition.z };

		pos.x = rotatedPosition.x;
		pos.y = rotatedPosition.y;
		pos.y = rotatedPosition.z;

		pLight.lightPosition = pos;
	}
}

void Light::SetColor(glm::vec4 color_)
{
	color = color_;
	if (lightType == LightType::Direct) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightColor = color;
		dLight.lightColor = color;
	}
	else if (lightType == LightType::Point) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightColor = color;
		pLight.lightColor = color;
	}
}

void Light::SetAmbientStrength(float amount)
{
	ambient = amount;
	if (lightType == LightType::Direct) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].ambientStrength = ambient;
		dLight.ambientStrength = ambient;
	}
	else if (lightType == LightType::Point) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].ambientStrength = ambient;
		pLight.ambientStrength = ambient;
	}
}

void Light::SetSpecularStrength(float amount)
{
	specular = amount;
	if (lightType == LightType::Direct) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].specularStrength = specular;
		dLight.specularStrength = specular;
	}
	else if (lightType == LightType::Point) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].specularStrength = specular;
		pLight.specularStrength = specular;
	}
}
