# Launcher
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Project)

set(VK_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/common_vk/include")
include_directories(${VK_INC_DIR})

set(ENGINE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/source")
set(ENGINE_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include")
include_directories(${ENGINE_INC_DIR})

set(GAME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/source")
set(GAME_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Game/include")
include_directories(${GAME_INC_DIR})

add_subdirectory(Engine)
add_subdirectory(Game)

FILE(GLOB_RECURSE SOURCES "${ENGINE_SRC_DIR}/*.cpp" "${GAME_SRC_DIR}/*.cpp")
FILE(GLOB_RECURSE INCLUDES "${ENGINE_INC_DIR}/*.hpp" "${GAME_INC_DIR}/*.hpp")

file(GLOB_RECURSE ENGINE_SRC
    "./Engine/include/*.hpp"
    "./Engine/source/*.cpp"
)
file(GLOB_RECURSE GAME_SRC
    "./Game/include/*.hpp"
    "./Game/source/*.cpp"
)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${INCLUDES})

source_group("Engine"            FILES ${ENGINE_SRC})
source_group("Game"            FILES ${GAME_SRC})
#target_link_libraries(${PROJECT_NAME} CubeEngine)

# common_vk
set(VK_DIR "./Engine/common_vk")
add_subdirectory("${VK_DIR}")
target_link_libraries(${PROJECT_NAME} common_vk)
target_include_directories(${PROJECT_NAME} PUBLIC ${VK_INC_DIR})

# SDL2
set(SDL_DIR "./Engine/libs/SDL2")
add_subdirectory("${SDL_DIR}")
target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
file(COPY ./Engine/libs/SDL2d.dll
DESTINATION ${CMAKE_BINARY_DIR}
)
#target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
#find_package(SDL2 REQUIRED)
#include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

# glm
set(GLM_DIR "./Engine/libs/glm")
add_subdirectory("${GLM_DIR}")
target_link_libraries(${PROJECT_NAME} glm)
#find_package(GLM REQUIRED)
#include_directories(${PROJECT_NAME} ${GLM_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} ${GLM_LIBRARIES})