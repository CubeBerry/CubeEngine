//Author: JEYOON YU
//Project: CubeEngine
//File: WorkGraphs.cpp
#include "DXRenderManager.hpp"

void DXRenderManager::InitializeWorkGraphs()
{
	m_workGraphsStateObject = std::make_unique<DXWorkGraphsStateObject>(m_device, "../Engine/shaders/cso/WorkGraphs.cso");
}

void DXRenderManager::ExecuteWorkGraphs()
{
}

void DXRenderManager::PrintWorkGraphsResults()
{
}