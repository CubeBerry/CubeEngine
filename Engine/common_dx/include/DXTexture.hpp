//Author: JEYOON YU
//Project: CubeEngine
//File: DXTexture.hpp
#pragma once
#pragma comment(lib, "dxguid.lib")
#include <directx/d3dx12.h>

#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

using Microsoft::WRL::ComPtr;

class DXTexture
{
public:
	DXTexture() = default;
	~DXTexture() = default;

	void LoadTexture(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12DescriptorHeap>& srvHeap,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const ComPtr<ID3D12Fence>& fence,
		const HANDLE& fenceEvent,
		bool isHDR, const std::filesystem::path& path_, std::string name_, bool flip);
	void LoadSkyBox(
		bool isHDR,
		const std::filesystem::path& right,
		const std::filesystem::path& left,
		const std::filesystem::path& top,
		const std::filesystem::path& bottom,
		const std::filesystem::path& front,
		const std::filesystem::path& back
	);
	void SetTextureID(const int id) { texID = id; }

	[[nodiscard]] int GetWidth() const { return width; }
	[[nodiscard]] int GetHeight() const { return height; }
	[[nodiscard]] glm::vec2 GetSize() const { return glm::vec2{ width, height }; }
	[[nodiscard]] std::string GetName() const { return name; }
	[[nodiscard]] int GetTextrueId() const { return texID; }
private:
	ComPtr<ID3D12Resource> m_texture;

	int width, height;
	int texID;
	std::string name;

	void FlipTextureHorizontally(uint8_t* src, int width_, int height_, int numComponents) const
	{
		for (int row = 0; row < height_; ++row)
		{
			for (int column = 0; column < width_ / 2; ++column)
			{
				int src_row{ row };
				int src_column{ column };
				int dest_column = width_ - 1 - src_column;

				int src_index{ (src_row * width_ + src_column) * numComponents };
				int dest_index{ (src_row * width_ + dest_column) * numComponents };

				for (int component = 0; component < numComponents; ++component)
					std::swap(src[src_index + component], src[dest_index + component]);
			}
		}
	};
};