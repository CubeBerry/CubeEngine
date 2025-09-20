# common_dx
cmake_minimum_required(VERSION 3.20)

set(DX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(DX_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
# FidelityFX 1.1.4
set(FFX_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libs/FidelityFX-SDK-v1.1.4/sdk/include")
# FidelityFX 2.0
# set(FFX_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libs/FidelityFX-SDK-v2.0.0")

FILE(GLOB DX_SOURCES "${DX_SRC_DIR}/*.cpp")
FILE(GLOB DX_INCLUDES "${DX_INC_DIR}/*.hpp")

if (MSVC)
    add_compile_options(/W4)
    add_compile_options(/MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
# set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_library(common_dx STATIC ${DX_SOURCES} ${DX_INCLUDES})

target_include_directories(common_dx PRIVATE ${DX_INC_DIR})
target_include_directories(common_dx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
target_include_directories(common_dx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/SDL3-3.2.16/include)
target_include_directories(common_dx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/imgui-1.91.9b)
target_include_directories(common_dx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009/include)
target_include_directories(common_dx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/DebugTools)
target_include_directories(common_dx PRIVATE ${FFX_INC_DIR})

# FetchContent for DirectX-Headers
include(FetchContent)
FetchContent_Declare(
    DirectXHeaders
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG main
)
FetchContent_MakeAvailable(DirectXHeaders)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(D3D12)
if(NOT D3D12_FOUND)
    message(FATAL_ERROR "Could not find D3D12. Please ensure the Windows SDK is installed.")
endif()

target_link_libraries(common_dx PUBLIC DirectX-Headers)
target_link_libraries(common_dx PRIVATE ${D3D12_LIBRARIES})