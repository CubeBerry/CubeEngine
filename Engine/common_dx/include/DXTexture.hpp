//Author: JEYOON YU
//Project: CubeEngine
//File: DXTexture.hpp
#pragma once
#pragma comment(lib, "dxguid.lib")
#include <d3dx12/d3dx12.h>

#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <functional>

using Microsoft::WRL::ComPtr;

class DXTexture
{
public:
	DXTexture() = default;
	~DXTexture();

	void LoadTexture(
		const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12CommandQueue>& commandQueue,
		const std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT>& srvHandle,
		std::function<void(UINT)> deallocator,
		const ComPtr<ID3D12Fence>& fence,
		const HANDLE& fenceEvent,
		bool isHDR, const std::filesystem::path& path_, const std::string& name_, bool flip);
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

	std::pair<CD3DX12_CPU_DESCRIPTOR_HANDLE, UINT> m_srvHandle;
	std::function<void(UINT)> m_deallocator;

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