# Launcher
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Project LANGUAGES CXX C)

# common_vk
set(VK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_vk")
add_subdirectory(${VK_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE ${VK_DIR}/include)

# common_gl
set(GL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_gl")
add_subdirectory(${GL_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE ${GL_DIR}/include)

# ImGui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/imgui")
set(IMGUI_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/imgui/backends")
# target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR})

# common libs (FMOD, ImGUI, stb)
set(COMMON_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs")

# FMOD
set(FMOD_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/fmod/inc")
set(FMOD_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/fmod/lib")
# target_include_directories(${PROJECT_NAME} PRIVATE ${FMOD_INC_DIR})

# Engine
set(ENGINE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/source")
set(ENGINE_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include")
# target_include_directories(${PROJECT_NAME} PRIVATE ${ENGINE_INC_DIR})

# Game
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/source")
set(GAME_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/include")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
# target_include_directories(${PROJECT_NAME} PRIVATE ${GAME_INC_DIR} ${ASSET_DIR})

file(GLOB ENGINE_SRC
    "${ENGINE_INC_DIR}/*.hpp"
    "${ENGINE_SRC_DIR}/*.cpp"
    "${ENGINE_INC_DIR}/BasicComponents/*.hpp"
    "${ENGINE_SRC_DIR}/BasicComponents/*.cpp"
    "${ENGINE_INC_DIR}/Particle/*.hpp"
    "${ENGINE_SRC_DIR}/Particle/*.cpp"
)
file(GLOB GAME_SRC
    "${GAME_INC_DIR}/*.hpp"
    "${GAME_SRC_DIR}/*.cpp"
    "${GAME_INC_DIR}/PlatformDemo/*.hpp"
    "${GAME_SRC_DIR}/PlatformDemo/*.cpp"
    "${GAME_INC_DIR}/PocketBallDemo/*.hpp"
    "${GAME_SRC_DIR}/PocketBallDemo/*.cpp"
    "${GAME_INC_DIR}/BeatEmUpDemo/*.hpp"
    "${GAME_SRC_DIR}/BeatEmUpDemo/*.cpp"
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
target_link_libraries(${PROJECT_NAME} PRIVATE ${VK_DIR}/libs/VulkanSDK/1.3.268.0/Lib/vulkan-1.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${VK_DIR}/libs/VulkanSDK/1.3.268.0/Lib/SDL2.lib)
# target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_DIR}/libs/glew/glew32.dll)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_DIR}/libs/glew/glew32.lib)
# For MSVC
target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

target_include_directories(${PROJECT_NAME} PRIVATE ${VK_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GL_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} ${IMGUI_BACKEND_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FMOD_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${ENGINE_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GAME_INC_DIR} ${ASSET_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR} ${COMMON_LIB_DIR}/SDL2)

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