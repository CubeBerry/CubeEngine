//Author: JEYOON YU
//Project: CubeEngine
//File: IRenderContext.hpp
#pragma once
#include "Interface/ICommandListWrapper.hpp"

class IRenderContext
{
public:
	virtual ~IRenderContext() = default;

	virtual void Initialize() = 0;
	virtual void OnResize() = 0;
	virtual void Execute(ICommandListWrapper* commandListWrapper) = 0;
	virtual void CleanUp() = 0;
};
