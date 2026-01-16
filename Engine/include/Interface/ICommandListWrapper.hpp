//Author: JEYOON YU
//Project: CubeEngine
//File: ICommandListWrapper.hpp
#pragma once

class ICommandListWrapper
{
public:
	virtual ~ICommandListWrapper() = default;
	virtual void* GetNativeHandle() const = 0;
};
