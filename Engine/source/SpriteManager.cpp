//Author: DOYEONG LEE
//Second Author: JEYOON YU
//Project: CubeEngine
//File: SpriteManager.cpp
#include "SpriteManager.hpp"
#include "BasicComponents/DynamicSprite.hpp"
#include "BasicComponents/StaticSprite.hpp"
#include "Engine.hpp"
#include "DXRenderManager.hpp"

SpriteManager::~SpriteManager()
{
	//if (sprites.empty() != true)
	//{
	//	auto iterator = sprites.begin();
	//	for (auto it = iterator; it != sprites.end(); ++it)
	//	{
	//		//Engine::Instance().GetVKRenderManager().DeleteWithIndex();
	//		sprites.erase(iterator);
	//		std::cout << sprites.size() << '\n'';
	//	}
	//}
    Engine::GetLogger().LogDebug(LogCategory::Engine, "Sprite Manager Deleted");
}

void SpriteManager::Update(float /*dt*/)
{
	//for (int i = 0; i < sprites.size(); i++)
	//{
	//	//sprites[i]->Update(dt);
	//	sprites[i]->Update(dt, sprites[i]->GetMaterialId());
	//}
}

void SpriteManager::End()
{
}

void SpriteManager::AddDynamicSprite(DynamicSprite* sprite)
{
    dynamicSprites.push_back(sprite);
}

void SpriteManager::DeleteDynamicSprite(DynamicSprite* sprite)
{
    if (!dynamicSprites.empty())
    {
        //auto iterator = std::find(sprites.begin(), sprites.end(), sprite_);
        auto iterator = std::ranges::find(dynamicSprites.begin(), dynamicSprites.end(), sprite);
        //for (auto it = iterator + 1; it != sprites.end(); ++it)
        //{
            //(*it)->SetMaterialId((*it)->GetMaterialId() - 1);

            //switch(Engine::Instance().GetRenderManager()->GetRenderType())
            //{
            //case RenderType::TwoDimension:
            //    Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetVertexUniforms2D()->at((*it)->GetMaterialId() + 1);
            //    Engine::Instance().GetRenderManager()->GetFragmentUniforms2D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetFragmentUniforms2D()->at((*it)->GetMaterialId() + 1);
            //    break;
            //case RenderType::ThreeDimension:
            //    Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetVertexUniforms3D()->at((*it)->GetMaterialId() + 1);
            //    Engine::Instance().GetRenderManager()->GetFragmentUniforms3D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetFragmentUniforms3D()->at((*it)->GetMaterialId() + 1);
            //    Engine::Instance().GetRenderManager()->GetMaterialUniforms3D()->at((*it)->GetMaterialId()) = Engine::Instance().GetRenderManager()->GetMaterialUniforms3D()->at((*it)->GetMaterialId() + 1);
            //    break;
            //}
        //}
        //Engine::Instance().GetRenderManager()->DeleteWithIndex(0);
        if (iterator != dynamicSprites.end())
        {
            //delete sprite_;
            dynamicSprites.erase(iterator);
        }
    }
}

// This must be called after loading whole mesh data into BufferWrapper
void SpriteManager::RegisterStaticSprite()
{
	SubMesh tempSubMesh;
	tempSubMesh = std::make_unique<BufferWrapper>(SpriteType::STATIC, true);

	auto* tempSprite = tempSubMesh->GetData<BufferWrapper::StaticSprite3D>();

	uint32_t currentMeshletOffset = 0;
	uint32_t currentVertexIndexOffset = 0;
	uint32_t currentPrimitiveIndexOffset = 0;
	uint32_t currentTransformIndex = 0;

	auto& tempVertices = tempSprite->vertices;
	auto& tempIndices = tempSprite->indices;
	auto& tempMeshlets = tempSprite->meshlets;
	auto& tempUniqueVertexIndices = tempSprite->uniqueVertexIndices;
	auto& tempPrimitiveIndices = tempSprite->primitiveIndices;

	std::vector<ThreeDimension::VertexUniform> vertexUniforms;
	std::vector<ThreeDimension::FragmentUniform> fragmentUniforms;
	std::vector<ThreeDimension::Material> materials;

	// @TODO Need to destroy original static sprites' buffers to save memory
    auto& objectMap = Engine::GetObjectManager().GetObjectMap();
    for (const auto& object : objectMap)
    {
        if (object.second->HasComponent<StaticSprite>())
        {
            StaticSprite* sprite = object.second->GetComponent<StaticSprite>();
			sprite->ClearMeshNodeRecords();

			auto& subMeshes = sprite->GetSubMeshes();
			// Store each submesh buffer data into one buffer
			for (auto& subMesh : subMeshes)
			{
				// Gather DynamicSprite3DMesh data and convert to StaticSprite3D data
				auto* spriteData = subMesh->GetData<BufferWrapper::DynamicSprite3DMesh>();

				tempVertices.insert(tempVertices.end(),
					spriteData->vertices.begin(),
					spriteData->vertices.end());
				tempMeshlets.insert(tempMeshlets.end(),
					spriteData->meshlets.begin(),
					spriteData->meshlets.end());
				tempUniqueVertexIndices.insert(tempUniqueVertexIndices.end(),
					spriteData->uniqueVertexIndices.begin(),
					spriteData->uniqueVertexIndices.end());
				tempPrimitiveIndices.insert(tempPrimitiveIndices.end(),
					spriteData->primitiveIndices.begin(),
					spriteData->primitiveIndices.end());
				tempIndices.insert(
					tempIndices.end(),
					spriteData->indices.begin(),
					spriteData->indices.end());

				vertexUniforms.push_back(spriteData->vertexUniform);
				fragmentUniforms.push_back(spriteData->fragmentUniform);
				materials.push_back(spriteData->material);

				StaticSprite::MeshNodeRecord record;
				record.objectID = currentTransformIndex;
				record.meshletOffset = currentMeshletOffset;
				record.meshletCount = static_cast<uint32_t>(spriteData->meshlets.size());
				record.vertexIndexOffset = currentVertexIndexOffset;
				record.primitiveIndexOffset = currentPrimitiveIndexOffset;

				sprite->AddMeshNodeRecord(record);
				sprite->SetGlobalTransformIndex(currentTransformIndex);

				currentMeshletOffset += record.meshletCount;
				currentVertexIndexOffset += static_cast<uint32_t>(spriteData->uniqueVertexIndices.size());
				currentPrimitiveIndexOffset += static_cast<uint32_t>(spriteData->primitiveIndices.size());
				currentTransformIndex++;
			}

			subMeshes.clear();
		}
    }

	// Initialize Buffers
	//staticSprite = std::make_unique<BufferWrapper>(SpriteType::STATIC, true);

	RenderManager* renderManager = Engine::Instance().GetRenderManager();
	dynamic_cast<DXRenderManager*>(renderManager)->InitializeStaticBuffers(*tempSubMesh, tempIndices);
	dynamic_cast<DXRenderManager*>(renderManager)->InitializeGlobalUniformBuffers(*tempSubMesh, vertexUniforms, fragmentUniforms, materials);
	globalStaticBuffer = std::move(tempSubMesh);

//
//	if (Engine::Instance().GetRenderManager()->GetGraphicsMode() == GraphicsMode::GL)
//	{
//		tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::QuantizedVertex) * quantizedVertices.size()), quantizedVertices.data());
//#ifdef _DEBUG
//		tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer->SetData(static_cast<GLsizei>(sizeof(ThreeDimension::NormalVertex) * normalVertices.size()), normalVertices.data());
//#endif
//
//		//Attributes
//		GLAttributeLayout position_layout;
//		position_layout.component_type = GLAttributeLayout::UInt;
//		position_layout.component_dimension = GLAttributeLayout::_1;
//		position_layout.normalized = false;
//		position_layout.vertex_layout_location = 0;
//		position_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
//		position_layout.offset = 0;
//		position_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, position);
//
//		GLAttributeLayout normal_layout;
//		normal_layout.component_type = GLAttributeLayout::Float;
//		normal_layout.component_dimension = GLAttributeLayout::_3;
//		normal_layout.normalized = false;
//		normal_layout.vertex_layout_location = 1;
//		normal_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
//		normal_layout.offset = 0;
//		normal_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, normal);
//
//		GLAttributeLayout uv_layout;
//		uv_layout.component_type = GLAttributeLayout::Float;
//		uv_layout.component_dimension = GLAttributeLayout::_2;
//		uv_layout.normalized = false;
//		uv_layout.vertex_layout_location = 2;
//		uv_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
//		uv_layout.offset = 0;
//		uv_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, uv);
//
//		GLAttributeLayout tex_sub_index_layout;
//		tex_sub_index_layout.component_type = GLAttributeLayout::Int;
//		tex_sub_index_layout.component_dimension = GLAttributeLayout::_1;
//		tex_sub_index_layout.normalized = false;
//		tex_sub_index_layout.vertex_layout_location = 3;
//		tex_sub_index_layout.stride = sizeof(ThreeDimension::QuantizedVertex);
//		tex_sub_index_layout.offset = 0;
//		tex_sub_index_layout.relative_offset = offsetof(ThreeDimension::QuantizedVertex, texSubIndex);
//
//		tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexArray->AddVertexBuffer(std::move(*tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexBuffer), sizeof(ThreeDimension::QuantizedVertex), { position_layout, normal_layout, uv_layout, tex_sub_index_layout });
//		tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().vertexArray->SetIndexBuffer(std::move(*tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().indexBuffer));
//
//#ifdef _DEBUG
//		GLAttributeLayout normal_position_layout;
//		normal_position_layout.component_type = GLAttributeLayout::Float;
//		normal_position_layout.component_dimension = GLAttributeLayout::_3;
//		normal_position_layout.normalized = false;
//		normal_position_layout.vertex_layout_location = 0;
//		normal_position_layout.stride = sizeof(ThreeDimension::NormalVertex);
//		normal_position_layout.offset = 0;
//		normal_position_layout.relative_offset = offsetof(ThreeDimension::NormalVertex, position);
//
//		tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexArray->AddVertexBuffer(std::move(*tempSubMesh->GetBuffer<BufferWrapper::GLBuffer>().normalVertexBuffer), sizeof(ThreeDimension::NormalVertex), { normal_position_layout });
//#endif
//	}
}
