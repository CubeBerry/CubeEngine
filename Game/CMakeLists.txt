# Game
cmake_minimum_required(VERSION 3.20)

project(Game)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
include_directories(${SRC_DIR} ${INC_DIR} ${ASSET_DIR})

set(ENGINE_INC_DIR "../Engine/include")
include_directories(${ENGINE_INC_DIR})

FILE(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
FILE(GLOB_RECURSE INCLUDES "${INC_DIR}/*.hpp")

#add_executable (Game ${SOURCES} ${INCLUDES})

# Engine
#set(ENGINE_DIR "../Engine")
#add_subdirectory("${ENGINE_DIR}")
#target_link_libraries(${PROJECT_NAME} "../Engine")
#target_include_directories(${PROJECT_NAME} PUBLIC ${ENGINE_INC_DIR})