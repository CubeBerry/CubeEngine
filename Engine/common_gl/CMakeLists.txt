# common_gl
cmake_minimum_required(VERSION 3.20)

set(GL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(GL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# include_directories(${SRC_DIR} ${INC_DIR} ${LIB_DIR})

FILE(GLOB GL_SOURCES "${GL_SRC_DIR}/*.cpp")
FILE(GLOB GL_INCLUDES "${GL_INC_DIR}/*.hpp")

add_library(common_gl STATIC ${GL_SOURCES} ${GL_INCLUDES})

target_include_directories(common_gl PRIVATE ${GL_INC_DIR})

target_include_directories(common_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
target_include_directories(common_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/imgui)
target_include_directories(common_gl PUBLIC ${LIB_DIR})