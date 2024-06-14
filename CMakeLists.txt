# Launcher
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Project LANGUAGES CXX C)

# common_vk
set(VK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_vk")
add_subdirectory("${VK_DIR}")
include_directories(${VK_DIR}/include)

# common_gl
set(GL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_gl")
add_subdirectory("${GL_DIR}")
include_directories(${GL_DIR}/include)

# ImGui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/imgui")
set(IMGUI_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/imgui/backends")
include_directories(${IMGUI_DIR})

# common libs (FMOD, ImGUI, stb)
set(COMMON_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs")

# FMOD
set(FMOD_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/fmod/inc")
set(FMOD_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/fmod/lib")
include_directories(${FMOD_INC_DIR})

# Engine
set(ENGINE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/source")
set(ENGINE_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include")
include_directories(${ENGINE_INC_DIR})

# Game
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/source")
set(GAME_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/include")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
include_directories(${GAME_INC_DIR} ${ASSET_DIR})

file(GLOB ENGINE_SRC
    "${ENGINE_INC_DIR}/*.hpp"
    "${ENGINE_SRC_DIR}/*.cpp"
)
file(GLOB GAME_SRC
    "${GAME_INC_DIR}/*.hpp"
    "${GAME_SRC_DIR}/*.cpp"
    "${GAME_INC_DIR}/PlatformDemo/*.hpp"
    "${GAME_SRC_DIR}/PlatformDemo/*.cpp"
    "${GAME_INC_DIR}/PocketBallDemo/*.hpp"
    "${GAME_SRC_DIR}/PocketBallDemo/*.cpp"
)
FILE(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_BACKEND_DIR}/*.h"
    "${IMGUI_BACKEND_DIR}/*.cpp"
)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_executable(${PROJECT_NAME} main.cpp ${ENGINE_SRC} ${GAME_SRC} ${IMGUI_SOURCES})

source_group("Engine"           FILES ${ENGINE_SRC})
source_group("Game"             FILES ${GAME_SRC})
source_group("ImGui"            FILES ${IMGUI_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE common_gl PRIVATE common_vk)

file(COPY ./Engine/common_gl/libs/glew/glew32.dll
DESTINATION ${CMAKE_BINARY_DIR}
)

# SDL2
#set(SDL_DIR "./Engine/libs/SDL2")
#add_subdirectory("${SDL_DIR}")
#target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
#file(COPY ${Vulkan_INCLUDE_DIR}/../Lib/SDL2.lib
#DESTINATION ${CMAKE_BINARY_DIR}
#)

# glm
#set(GLM_DIR "./Engine/libs/glm")
#add_subdirectory("${GLM_DIR}")
#target_link_libraries(${PROJECT_NAME} glm)

# FMOD
file(COPY ./Engine/libs/fmod/lib/fmod.dll
DESTINATION ${CMAKE_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${FMOD_LIB_DIR}/fmod_vc.lib)