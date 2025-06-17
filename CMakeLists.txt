# Launcher
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Project LANGUAGES CXX C)

# Set Directories
# common_gl
set(GL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_gl")
# common_vk
set(VK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_vk")
# common_dx
set(DX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_dx")
# ImGui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/imgui-1.91.9b")
set(IMGUI_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/imgui-1.91.9b/backends")
# common libs (FMOD, ImGUI, stb, SDL, glm, assimp)
set(COMMON_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs")
# FMOD
set(FMOD_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/fmod/inc")
set(FMOD_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/fmod/lib")
# Assimp
set(ASSIMP_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/assimp/include")
# Engine
set(ENGINE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/source")
set(ENGINE_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include")
# Game
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/source")
set(GAME_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/include")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

# Subdirectories
# common_gl
add_subdirectory(${GL_DIR})
# common_vk
add_subdirectory(${VK_DIR})
# common_dx
add_subdirectory(${DX_DIR})
# SDL3
add_subdirectory(${COMMON_LIB_DIR}/SDL3-3.2.16)

# Compile Settings
FILE(GLOB ENGINE_SRC
    "${ENGINE_INC_DIR}/*.hpp"
    "${ENGINE_SRC_DIR}/*.cpp"
    "${ENGINE_INC_DIR}/BasicComponents/*.hpp"
    "${ENGINE_SRC_DIR}/BasicComponents/*.cpp"
    "${ENGINE_INC_DIR}/Particle/*.hpp"
    "${ENGINE_SRC_DIR}/Particle/*.cpp"
)
FILE(GLOB GAME_SRC
    "${GAME_INC_DIR}/*.hpp"
    "${GAME_SRC_DIR}/*.cpp"
    "${GAME_INC_DIR}/PlatformDemo/*.hpp"
    "${GAME_SRC_DIR}/PlatformDemo/*.cpp"
    "${GAME_INC_DIR}/PocketBallDemo/*.hpp"
    "${GAME_SRC_DIR}/PocketBallDemo/*.cpp"
    "${GAME_INC_DIR}/BeatEmUpDemo/*.hpp"
    "${GAME_SRC_DIR}/BeatEmUpDemo/*.cpp"
)
FILE(GLOB IMGUI_SRC
    "${IMGUI_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_BACKEND_DIR}/*.h"
    "${IMGUI_BACKEND_DIR}/*.cpp"
)

if (MSVC)
    add_compile_options(/W4)
    add_compile_options(/MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
# Turnn off warning as error due to ImGui DX12
# set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_executable(${PROJECT_NAME} main.cpp DebugTools.hpp ${ENGINE_SRC} ${GAME_SRC} ${IMGUI_SRC})

source_group("Engine"           FILES ${ENGINE_SRC})
source_group("Game"             FILES ${GAME_SRC})
source_group("ImGui"            FILES ${IMGUI_SRC})

# Copy DLL Files
FILE(COPY ./Engine/common_gl/libs/glew/glew32.dll
DESTINATION ${CMAKE_BINARY_DIR}
)
FILE(COPY ./Engine/libs/SDL3-3.2.16/SDL3.dll
DESTINATION ${CMAKE_BINARY_DIR}
)
# FMOD
FILE(COPY ./Engine/libs/fmod/lib/fmod.dll
DESTINATION ${CMAKE_BINARY_DIR}
)
# Assimp
FILE(COPY ./Engine/libs/assimp/assimp-vc143-mtd.dll
DESTINATION ${CMAKE_BINARY_DIR}
)

# Link Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE common_gl PRIVATE common_vk PRIVATE common_dx)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_DIR}/libs/glew/glew32.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${FMOD_LIB_DIR}/fmod_vc.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR}/assimp/assimp-vc143-mtd.lib)
# For MSVC
target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

# Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE ${GL_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${VK_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${DX_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} ${IMGUI_BACKEND_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FMOD_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${ENGINE_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GAME_INC_DIR} ${ASSET_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR} ${COMMON_LIB_DIR}/SDL3-3.2.16/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GL_DIR}/libs)
