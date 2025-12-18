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
# FidelityFX 1.1.4
set(FFX_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/FidelityFX-SDK-v1.1.4/sdk/include")
# FidelityFX 2.0
# set(FFX_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/FidelityFX-SDK-v2.0.0")
# meshoptimizer
set(MESHOPTIMIZER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/meshoptimizer")
set(MESHOPTIMIZER_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/libs/meshoptimizer/src")
# Engine
set(ENGINE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/source")
set(ENGINE_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include")
# Game
set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/source")
set(GAME_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/include")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
# DirectX 12 Agility SDK
# Turn on/off DirectX 12 Agility SDK Preview Version
option(USE_PREVIEW_SDK "Enable DirectX 12 Agility SDK Preview Version (715)" OFF)
if(USE_PREVIEW_SDK)
# DirectX 12 Agility SDK 1.715.0-preview or later is required for Mesh Nodes
set(AGILITY_SDK_VERSION "1.715.0-preview")
add_compile_definitions(USE_PREVIEW_SDK=1)
else()
set(AGILITY_SDK_VERSION "1.618.4")
add_compile_definitions(USE_PREVIEW_SDK=0)
endif()

# CMAKE_BINARY_DIR is used to locate directory where files are built
set(NUGET_PACKAGES_ROOT "${CMAKE_BINARY_DIR}/packages")

# Copy packages.config to build folder for NuGet package restore (for DirectX 12 Agility SDK)
# configure_file(packages.config ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)

set(NUGET_EXE "${CMAKE_CURRENT_SOURCE_DIR}/Tools/nuget.exe")
if (NOT EXISTS "${NUGET_EXE}")
    message(FATAL_ERROR "nuget.exe not found at ${NUGET_EXE}. Please download it from https://www.nuget.org/downloads and place it in the 'Tools' directory under the project root.")
endif()

message(STATUS "Installing NuGet packages...")

execute_process(
    COMMAND ${NUGET_EXE} install Microsoft.Direct3D.D3D12 -OutputDirectory ${NUGET_PACKAGES_ROOT} -Version ${AGILITY_SDK_VERSION}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE NUGET_RESULT
)

if (NOT NUGET_RESULT EQUAL 0)
    message(FATAL_ERROR "NuGet restore failed. Please ensure 'nuget.exe' is in your PATH.")
endif()

set(AGILITY_SDK_BIN_DIR "${NUGET_PACKAGES_ROOT}/Microsoft.Direct3D.D3D12.${AGILITY_SDK_VERSION}/build/native/bin/x64")
set(AGILITY_SDK_INCLUDE_DIR "${NUGET_PACKAGES_ROOT}/Microsoft.Direct3D.D3D12.${AGILITY_SDK_VERSION}/build/native/include")

# Subdirectories
# common_gl
add_subdirectory(${GL_DIR})
# common_vk
add_subdirectory(${VK_DIR})
# common_dx
add_subdirectory(${DX_DIR})
# SDL3
add_subdirectory(${COMMON_LIB_DIR}/SDL3-3.2.16)
# meshoptimizer
add_subdirectory(${MESHOPTIMIZER_DIR})

# Compile Settings
FILE(GLOB DEBUG_TOOLS_SRC
    "${ENGINE_INC_DIR}/DebugTools/*.h"
    "${ENGINE_INC_DIR}/DebugTools/*.hpp"
    "${ENGINE_SRC_DIR}/DebugTools/*.cpp"
)
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

add_executable(${PROJECT_NAME} main.cpp ${DEBUG_TOOLS_SRC} ${ENGINE_SRC} ${GAME_SRC} ${IMGUI_SRC})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${AGILITY_SDK_BIN_DIR}/D3D12Core.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${AGILITY_SDK_BIN_DIR}/D3D12SDKLayers.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12/"
)

source_group("DebugTools"       FILES ${DEBUG_TOOLS_SRC})
source_group("Engine"           FILES ${ENGINE_SRC})
source_group("Game"             FILES ${GAME_SRC})
source_group("ImGui"            FILES ${IMGUI_SRC})

# Copy DLL Files
# glew
FILE(COPY ./Engine/common_gl/libs/glew/glew32.dll
DESTINATION ${CMAKE_BINARY_DIR}
)
# SDL3
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
# NVIDIA Aftermath
FILE(COPY ./Engine/libs/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009/lib/x64/GFSDK_Aftermath_Lib.x64.dll
DESTINATION ${CMAKE_BINARY_DIR}
)

# Link Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE common_gl PRIVATE common_vk PRIVATE common_dx)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_DIR}/libs/glew/glew32.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${FMOD_LIB_DIR}/fmod_vc.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR}/assimp/assimp-vc143-mtd.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR}/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009/lib/x64/GFSDK_Aftermath_Lib.x64.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE meshoptimizer)
# FidelityFX SDK 1.1.4's .lib files can be acquired by following steps:
# 1. Move to Engine/libs/FidelityFX-SDK-v1.1.4/sdk
# 1. Run BuildFidelityFXSDK.bat or BuildFidelityFXSDKSolutionDX12.bat or BuildFidelityFXSDKSolutionVK.bat
# 2. Run BuildFidelityFXSDK.bat or Run BuildFidelityFXSDKSolutionDX12.bat or BuildFidelityFXSDKSolutionVK.bat -> Open and build solution created in build folder
# 3. Must enter y (yes) to Auto-compile shaders [y/n]?
# 4. .lib files are created in bin folder
# Debug
target_link_libraries(${PROJECT_NAME} PRIVATE debug ${FFX_INC_DIR}/../bin/ffx_sdk/ffx_backend_dx12_x64d.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE debug ${FFX_INC_DIR}/../bin/ffx_sdk/ffx_cas_x64d.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE debug ${FFX_INC_DIR}/../bin/ffx_sdk/ffx_fsr1_x64d.lib)
# Release
target_link_libraries(${PROJECT_NAME} PRIVATE optimized ${FFX_INC_DIR}/../bin/ffx_sdk/ffx_backend_dx12_x64.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE optimized ${FFX_INC_DIR}/../bin/ffx_sdk/ffx_cas_x64.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE optimized ${FFX_INC_DIR}/../bin/ffx_sdk/ffx_fsr1_x64.lib)
# For MSVC
target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

# Include Directories
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${AGILITY_SDK_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GL_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${VK_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${DX_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} ${IMGUI_BACKEND_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FMOD_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FFX_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${ENGINE_INC_DIR} ${ENGINE_INC_DIR}/DebugTools)
target_include_directories(${PROJECT_NAME} PRIVATE ${GAME_INC_DIR} ${ASSET_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR} ${COMMON_LIB_DIR}/SDL3-3.2.16/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GL_DIR}/libs)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_LIB_DIR}/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${MESHOPTIMIZER_INC_DIR})
