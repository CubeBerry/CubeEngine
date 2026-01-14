//Author: DOYEONG LEE
//Project: CubeEngine
//File: Camera.hpp
#pragma once
#include "Utility.hpp"

#include <variant>
#include <type_traits>
#include <map>
#include <string>

#include "GLVertexArray.hpp"
#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"

#include "GLUniformBuffer.hpp"

#include "VKVertexBuffer.hpp"
#include "VKIndexBuffer.hpp"

#include "DXVertexBuffer.hpp"
#include "DXIndexBuffer.hpp"

#include "DXConstantBuffer.hpp"
#include "DXStructuredBuffer.hpp"

template<typename T, typename Variant>
struct is_variant_member;

template<typename T, typename... Ts>
struct is_variant_member<T, std::variant<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename Variant>
inline constexpr bool is_variant_member_v = is_variant_member<T, Variant>::value;

//class GLVertexArray;
//class GLVertexBuffer;
//class GLIndexBuffer;
//
//class VKVertexBuffer;
//class VKIndexBuffer;
//
//class DXVertexBuffer;
//class DXIndexBuffer;
//
//template<typename T>
//class GLUniformBuffer;
//
//template<typename T>
//class DXConstantBuffer;
//template<typename T>
//class DXStructuredBuffer;

enum class SpriteType
{
	STATIC,
	DYNAMIC
};

// Buffer
struct BufferWrapper
{
public:
	struct GLBuffer
	{
		std::unique_ptr<GLVertexArray> vertexArray;
		std::unique_ptr<GLVertexBuffer> vertexBuffer;
#ifdef _DEBUG
		std::unique_ptr<GLVertexArray> normalVertexArray;
		std::unique_ptr<GLVertexBuffer> normalVertexBuffer;
#endif
		std::unique_ptr<GLIndexBuffer> indexBuffer;
	};

	struct VKBuffer
	{
		std::unique_ptr<VKVertexBuffer> vertexBuffer;
#ifdef _DEBUG
		std::unique_ptr<VKVertexBuffer> normalVertexBuffer;
#endif
		std::unique_ptr<VKIndexBuffer> indexBuffer;
	};

	struct DXBuffer
	{
		std::unique_ptr<DXVertexBuffer> vertexBuffer;
#ifdef _DEBUG
		std::unique_ptr<DXVertexBuffer> normalVertexBuffer;
#endif
		std::unique_ptr<DXIndexBuffer> indexBuffer;

		std::unique_ptr<DXStructuredBuffer<ThreeDimension::QuantizedVertex>> uniqueVertexBuffer;
		std::unique_ptr<DXStructuredBuffer<ThreeDimension::StaticQuantizedVertex>> uniqueStaticVertexBuffer;
		std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT> srvHandle;
		std::unique_ptr<DXStructuredBuffer<Meshlet::Meshlet>> meshletBuffer;
		std::unique_ptr<DXStructuredBuffer<uint32_t>> uniqueVertexIndexBuffer;
		std::unique_ptr<DXStructuredBuffer<uint32_t>> primitiveIndexBuffer;
	};

	// @TODO Move uniform buffers to each GLBuffer, VKBuffer, DXBuffer structs if it's proper way
	struct DynamicSprite2D
	{
		// CPU Data
		std::vector<TwoDimension::Vertex> vertices;
		TwoDimension::VertexUniform vertexUniform;
		TwoDimension::FragmentUniform fragmentUniform;
		std::vector<uint32_t> indices;

		// GPU Buffers
		//std::variant<std::monostate, GLBuffer, VKBuffer, DXBuffer> buffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<TwoDimension::VertexUniform>>,
			std::unique_ptr<DXConstantBuffer<TwoDimension::VertexUniform>>
		> vertexUniformBuffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<TwoDimension::FragmentUniform>>,
			std::unique_ptr<DXConstantBuffer<TwoDimension::FragmentUniform>>
		> fragmentUniformBuffer;

		void Initialize();

		template<typename T>
		T* GetVertexUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(vertexUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in vertexUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&vertexUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		T* GetFragmentUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(fragmentUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in fragmentUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&fragmentUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		void SetVertexUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(vertexUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			vertexUniformBuffer = std::move(rbuffer);
		}

		template<typename T>
		void SetFragmentUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(fragmentUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			fragmentUniformBuffer = std::move(rbuffer);
		}
	};

	// @TODO Temporarily not used
//	struct DynamicSprite3D
//	{
//		// CPU Data
//		std::vector<ThreeDimension::QuantizedVertex> vertices;
//#ifdef _DEBUG
//		std::vector<ThreeDimension::NormalVertex> normalVertices;
//#endif
//		std::vector<uint32_t> indices;
//		ThreeDimension::VertexUniform vertexUniform;
//		ThreeDimension::FragmentUniform fragmentUniform;
//		ThreeDimension::Material material;
//
//		// GPU Buffers
//		//std::variant<std::monostate, GLBuffer, VKBuffer, DXBuffer> buffer;
//		std::variant<
//			std::monostate,
//			std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>,
//			std::unique_ptr<DXConstantBuffer<ThreeDimension::VertexUniform>>
//		> vertexUniformBuffer;
//		std::variant<
//			std::monostate,
//			std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>>,
//			std::unique_ptr<DXConstantBuffer<ThreeDimension::FragmentUniform>>
//		> fragmentUniformBuffer;
//		std::variant<
//			std::monostate,
//			std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>>,
//			std::unique_ptr<DXConstantBuffer<ThreeDimension::Material>>
//		> materialUniformBuffer;
//
//		void Initialize()
//		{
//			switch (Engine::GetRenderManager()->GetGraphicsMode())
//			{
//			case GraphicsMode::GL:
//				vertexUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>>();
//				fragmentUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>>>();
//				materialUniformBuffer.emplace<std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>>>();
//				break;
//			case GraphicsMode::DX:
//				vertexUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<ThreeDimension::VertexUniform>>>();
//				fragmentUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<ThreeDimension::FragmentUniform>>>();
//				materialUniformBuffer.emplace<std::unique_ptr<DXConstantBuffer<ThreeDimension::Material>>>();
//				break;
//			}
//		}
//
//		template<typename T>
//		T* GetVertexUniformBuffer()
//		{
//			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&vertexUniformBuffer);
//			return bufferPtr ? bufferPtr->get() : nullptr;
//		}
//
//		template<typename T>
//		T* GetFragmentUniformBuffer()
//		{
//			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&fragmentUniformBuffer);
//			return bufferPtr ? bufferPtr->get() : nullptr;
//		}
//
//		template<typename T>
//		T* GetMaterialUniformBuffer()
//		{
//			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&materialUniformBuffer);
//			return bufferPtr ? bufferPtr->get() : nullptr;
//		}
//
//		template<typename T>
//		void SetVertexUniformBuffer(std::unique_ptr<T>&& buffer)
//		{
//			vertexUniformBuffer = std::move(buffer);
//		}
//
//		template<typename T>
//		void SetFragmentUniformBuffer(std::unique_ptr<T>&& buffer)
//		{
//			fragmentUniformBuffer = std::move(buffer);
//		}
//
//		template<typename T>
//		void SetMaterialUniformBuffer(std::unique_ptr<T>&& buffer)
//		{
//			materialUniformBuffer = std::move(buffer);
//		}
//	};

	// @TODO Temporarily used for both non-mesh shader and mesh shader
	struct DynamicSprite3DMesh
	{
		// CPU Data
		std::vector<ThreeDimension::QuantizedVertex> vertices;
#ifdef _DEBUG
		std::vector<ThreeDimension::NormalVertex> normalVertices;
#endif
		std::vector<uint32_t> indices;

		std::vector<Meshlet::Meshlet> meshlets;
		std::vector<uint32_t> uniqueVertexIndices;
		std::vector<uint32_t> primitiveIndices;

		ThreeDimension::VertexUniform vertexUniform;
		ThreeDimension::FragmentUniform fragmentUniform;
		ThreeDimension::Material material;

		// Bone Information Storage (Replacing Model class functionality)
		// Stores the mapping from bone name to BoneInfo (ID and Offset Matrix)
		std::map<std::string, ThreeDimension::BoneInfo> boneInfoMap;
		int boneCount = 0;

		// GPU Buffers
		//std::variant<std::monostate, GLBuffer, VKBuffer, DXBuffer> buffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>,
			std::unique_ptr<DXConstantBuffer<ThreeDimension::VertexUniform>>
		> vertexUniformBuffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>>,
			std::unique_ptr<DXConstantBuffer<ThreeDimension::FragmentUniform>>
		> fragmentUniformBuffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>>,
			std::unique_ptr<DXConstantBuffer<ThreeDimension::Material>>
		> materialUniformBuffer;

		void Initialize();

		template<typename T>
		T* GetVertexUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(vertexUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in vertexUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&vertexUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		T* GetFragmentUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(fragmentUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in fragmentUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&fragmentUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		T* GetMaterialUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(materialUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in materialUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&materialUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		void SetVertexUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(vertexUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			vertexUniformBuffer = std::move(rbuffer);
		}

		template<typename T>
		void SetFragmentUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(fragmentUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			fragmentUniformBuffer = std::move(rbuffer);
		}

		template<typename T>
		void SetMaterialUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(materialUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			materialUniformBuffer = std::move(rbuffer);
		}
	};

	struct StaticSprite3D
	{
		// CPU Data (optional, can be cleared after upload)
		std::vector<ThreeDimension::StaticQuantizedVertex> vertices;
		std::vector<uint32_t> indices;

		std::vector<Meshlet::Meshlet> meshlets;
		std::vector<uint32_t> uniqueVertexIndices;
		std::vector<uint32_t> primitiveIndices;

		std::vector<ThreeDimension::VertexUniform> vertexUniforms;
		std::vector<ThreeDimension::FragmentUniform> fragmentUniforms;
		std::vector<ThreeDimension::Material> materials;

		// GPU Buffers
		//std::variant<std::monostate, GLBuffer, VKBuffer, DXBuffer> buffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<ThreeDimension::VertexUniform>>,
			std::unique_ptr<DXStructuredBuffer<ThreeDimension::VertexUniform>>
		> vertexUniformBuffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<ThreeDimension::FragmentUniform>>,
			std::unique_ptr<DXStructuredBuffer<ThreeDimension::FragmentUniform>>
		> fragmentUniformBuffer;
		std::variant<
			std::monostate,
			std::unique_ptr<GLUniformBuffer<ThreeDimension::Material>>,
			std::unique_ptr<DXStructuredBuffer<ThreeDimension::Material>>
		> materialUniformBuffer;

		void Initialize();

		template<typename T>
		T* GetVertexUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(vertexUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in vertexUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&vertexUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		T* GetFragmentUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(fragmentUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in fragmentUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&fragmentUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		T* GetMaterialUniformBuffer()
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(materialUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Template type T is NOT defined in materialUniformBuffer variant.");

			auto* bufferPtr = std::get_if<std::unique_ptr<T>>(&materialUniformBuffer);
			return bufferPtr ? bufferPtr->get() : nullptr;
		}

		template<typename T>
		void SetVertexUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(vertexUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			vertexUniformBuffer = std::move(rbuffer);
		}

		template<typename T>
		void SetFragmentUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(fragmentUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			fragmentUniformBuffer = std::move(rbuffer);
		}

		template<typename T>
		void SetMaterialUniformBuffer(std::unique_ptr<T>&& rbuffer)
		{
			using TargetType = std::unique_ptr<T>;
			using VariantType = decltype(materialUniformBuffer);
			static_assert(is_variant_member_v<TargetType, VariantType>,
				"ERROR: Trying to set a type that is NOT defined in the variant.");

			materialUniformBuffer = std::move(rbuffer);
		}
	};

	BufferWrapper(SpriteType spriteType, bool meshShaderEnabled = false);
	~BufferWrapper();

	BufferWrapper(const BufferWrapper&) = delete;
	BufferWrapper& operator=(const BufferWrapper&) = delete;
	BufferWrapper(BufferWrapper&&) = default;
	BufferWrapper& operator=(BufferWrapper&&) = default;

	//template<typename T>
	//const T* GetBuffer() const
	//{
	//	return std::get_if<T>(&buffer);
	//}

	template<typename T>
	T* GetBuffer()
	{
		return std::get_if<T>(&buffer);
	}

	//template<typename T>
	//const T* GetData() const
	//{
	//	return std::get_if<T>(&data);
	//}

	template<typename T>
	T* GetData()
	{
		return std::get_if<T>(&data);
	}

	//RenderType GetRenderType() const {
	//	if (std::holds_alternative<DynamicSprite2D>(data)) return RenderType::TwoDimension;
	//	return RenderType::ThreeDimension;
	//}

	SpriteType GetSpriteType() const {
		if (std::holds_alternative<DynamicSprite3DMesh>(data)) return SpriteType::DYNAMIC;
		return SpriteType::STATIC;
	}
private:
	// @TODO Need to know mesh shader is used or not
	// Dynamic/Sprite can be known because BufferWrapper is initialized in DynamicSprite or StaticSprite creation function
	//friend class RenderManager;

	std::variant<std::monostate, GLBuffer, VKBuffer, DXBuffer> buffer;
	std::variant<
		std::monostate,
		DynamicSprite2D,
		//DynamicSprite3D,
		DynamicSprite3DMesh,
		StaticSprite3D
	> data;
};

using SubMesh = std::unique_ptr<BufferWrapper>;