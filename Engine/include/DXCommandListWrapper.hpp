//Author: JEYOON YU
//Project: CubeEngine
//File: DXCommandListWrapper.hpp
#pragma once
#include "Interface/ICommandListWrapper.hpp"
#include <d3d12.h>

class DXCommandListWrapper : public ICommandListWrapper
{
public:
	DXCommandListWrapper(ID3D12GraphicsCommandList10* commandList) : m_commandList(commandList) {}
	void* GetNativeHandle() const override { return m_commandList; }
	ID3D12GraphicsCommandList10* GetDXCommandList() const { return m_commandList; }
private:
	ID3D12GraphicsCommandList10* m_commandList;
};
