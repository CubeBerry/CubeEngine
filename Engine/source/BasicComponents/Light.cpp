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
	if (pointLight != nullptr)
	{
		delete pointLight;
	}
}

void Light::Init()
{
}

void Light::AddLight(LightType lightType_, float ambient_, float specular_)
{
	if (lightType == LightType::NONE) 
	{
		lightType = lightType_;
		pos = GetOwner()->GetPosition();
		if (lightType == LightType::DIRECTIONAL)
		{
			dLight.lightDirection = { 0.f,0.f,0.f };
			dLight.ambientStrength = ambient_;
			dLight.specularStrength = specular_;
			Engine::GetRenderManager()->AddDirectionalLight(dLight);
			lightlId = static_cast<int>(Engine::GetRenderManager()->GetDirectionalLightUniforms().size() - 1);
		}
		else if (lightType == LightType::POINT)
		{
			pLight.ambientStrength = ambient_;
			pLight.specularStrength = specular_;
			Engine::GetRenderManager()->AddPointLight(pLight);
			lightlId = static_cast<int>(Engine::GetRenderManager()->GetPointLightUniforms().size() - 1);

			pointLight = new Sprite();
			pointLight->AddMesh3D(MeshType::CUBE, "", 1, 1, color);
		}
	}
}

void Light::Update(float /*dt*/)
{
	if(pointLight != nullptr)
	{
		pointLight->UpdateModel(GetOwner()->GetPosition() + pos, { 0.05f,0.05f,0.05f }, { rotate.x, rotate.y, rotate.z });
		pointLight->UpdateProjection();
		pointLight->UpdateView();
		//pointLight->SetColor(color);
	

		if (lightType == LightType::DIRECTIONAL) 
		{
			Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId] = dLight;
		}
		else if (lightType == LightType::POINT) 
		{
			Engine::GetRenderManager()->GetPointLightUniforms()[lightlId] = pLight;
			Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition = GetOwner()->GetPosition() + pos;

		}
	}
}

void Light::End()
{
}

void Light::SetXPosition(float x)
{
	pos.x = x;
	if (lightType == LightType::POINT) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.x = pos.x;
		pLight.lightPosition.x = pos.x;
	}
}

void Light::SetYPosition(float y)
{
	pos.y = y;
	if (lightType == LightType::POINT)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.y = pos.y;
		pLight.lightPosition.y = pos.y;
	}
}

void Light::SetZPosition(float z)
{
	pos.z = z;
	if (lightType == LightType::POINT)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.z = pos.z;
		pLight.lightPosition.z = pos.z;
	}
}

void Light::SetPosition(glm::vec3 pos_)
{
	pos = pos_;
	if (lightType == LightType::POINT)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.x = pos.x;
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.y = pos.y;
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightPosition.z = pos.z;


		pLight.lightPosition = pos;
	}
}

void Light::SetXRotate(float x)
{
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

	if (lightType == LightType::DIRECTIONAL)
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightDirection.x = glm::radians(-rotate.x);
		dLight.lightDirection.x = glm::radians(-rotate.x);
	}
	else if (lightType == LightType::POINT) 
	{
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

	if (lightType == LightType::DIRECTIONAL) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightDirection.y = glm::radians(-rotate.y);
		dLight.lightDirection.y = glm::radians(-rotate.y);
	}
	else if (lightType == LightType::POINT) 
	{
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

	if (lightType == LightType::DIRECTIONAL)
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightDirection.z = glm::radians(-rotate.z);
		dLight.lightDirection.z = glm::radians(-rotate.z);
	}
	else if (lightType == LightType::POINT) 
	{
	}
}

void Light::SetRotate(glm::vec3 rotate_)
{
	SetXRotate(rotate_.x);
	SetYRotate(rotate_.y);
	SetZRotate(rotate_.z);

	if (lightType == LightType::POINT) 
	{
	}
}

void Light::SetColor(glm::vec4 color_)
{
	color = color_;
	if (lightType == LightType::DIRECTIONAL) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].lightColor = color;
		dLight.lightColor = color;
	}
	else if (lightType == LightType::POINT) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].lightColor = color;
		pLight.lightColor = color;
	}
}

void Light::SetAmbientStrength(float amount)
{
	ambient = amount;
	if (lightType == LightType::DIRECTIONAL) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].ambientStrength = ambient;
		dLight.ambientStrength = ambient;
	}
	else if (lightType == LightType::POINT) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].ambientStrength = ambient;
		pLight.ambientStrength = ambient;
	}
}

void Light::SetSpecularStrength(float amount)
{
	specular = amount;
	if (lightType == LightType::DIRECTIONAL) 
	{
		Engine::GetRenderManager()->GetDirectionalLightUniforms()[lightlId].specularStrength = specular;
		dLight.specularStrength = specular;
	}
	else if (lightType == LightType::POINT) 
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].specularStrength = specular;
		pLight.specularStrength = specular;
	}
}

void Light::SetConstant(float amount)
{
	constant = amount;
	if (lightType == LightType::POINT)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].constant = constant;
		pLight.constant = constant;
	}
}

void Light::SetLinear(float amount)
{
	linear = amount;
	if (lightType == LightType::POINT)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].linear = linear;
		pLight.linear = linear;
	}
}

void Light::SetQuadratic(float amount)
{
	quadratic = amount;
	if (lightType == LightType::POINT)
	{
		Engine::GetRenderManager()->GetPointLightUniforms()[lightlId].quadratic = quadratic;
		pLight.quadratic = quadratic;
	}
}
