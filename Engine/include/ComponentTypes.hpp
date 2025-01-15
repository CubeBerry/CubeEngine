//Author: DOYEONG LEE
//Project: CubeEngine
//File: ComponentTypes.hpp
#pragma once
#include <string>

enum class ComponentTypes
{
    SPRITE = 0,
    PHYSICS2D,
    PHYSICS3D,
    LIGHT,
    INVALID 
};

inline ComponentTypes StringToComponent(const std::string& string)
{
    if (string == "SPRITE")
       return ComponentTypes::SPRITE;
    else if (string == "PHYSICS2D")
        return ComponentTypes::PHYSICS2D;
    else if (string == "PHYSICS3D")
        return ComponentTypes::PHYSICS3D;
    else if (string == "LIGHT")
        return ComponentTypes::LIGHT;

    return ComponentTypes::INVALID;
}

inline std::string ComponentToString(ComponentTypes type)
{
    if (type == ComponentTypes::SPRITE)
        return "SPRITE";
    else if (type == ComponentTypes::PHYSICS2D)
        return "PHYSICS2D";
    else if (type == ComponentTypes::PHYSICS3D)
        return "PHYSICS3D";
    else if (type == ComponentTypes::LIGHT)
        return "LIGHT";

    return "INVALID";
}