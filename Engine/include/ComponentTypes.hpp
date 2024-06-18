//Author: DOYEONG LEE
//Project: CubeEngine
//File: ComponentTypes.hpp
#pragma once
#include <string>

enum class ComponentTypes
{
    INVALID = 0,
    SPRITE,
    PHYSICS2D
};

inline ComponentTypes StringToComponent(const std::string& /*string*/)
{
    //if (string == "DemoUIButton")
    //   return eComponentTypes::DemoUIButton;
    return ComponentTypes::INVALID;
}

inline std::string ComponentToString(ComponentTypes /*type*/)
{
    //if (type == eComponentTypes::Player2BoxMove)
      //  return "Player2BoxMove";

    return "Invalid";
}