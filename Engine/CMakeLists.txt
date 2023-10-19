# CubeEngine
cmake_minimum_required(VERSION 3.20)
project(CubeEngine)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
# common_vk directories
#set(VK_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common_vk/source")
#set(VK_INC_DIR "./common_vk/include")
set(VK_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common_vk/include")
#set(VK_SRC_DIR "./common_vk/source")
#set(VK_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common_vk/libs")

#set(SDL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2")

include_directories(${SRC_DIR} ${INC_DIR} ${LIB_DIR} ${VK_INC_DIR})

FILE(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
FILE(GLOB_RECURSE INCLUDES "${INC_DIR}/*.hpp")

#add_library(CubeEngine STATIC ${SOURCES} ${INCLUDES})