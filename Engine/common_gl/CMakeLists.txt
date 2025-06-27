# common_gl
cmake_minimum_required(VERSION 3.20)

set(GL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(GL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

FILE(GLOB GL_SOURCES "${GL_SRC_DIR}/*.cpp")
FILE(GLOB GL_INCLUDES "${GL_INC_DIR}/*.hpp")

add_library(common_gl STATIC ${GL_SOURCES} ${GL_INCLUDES})

if (MSVC)
    add_compile_options(/W4)
    add_compile_options(/MP)
    target_compile_options(common_gl PRIVATE "/GH" "/Gh")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    target_compile_options(common_gl PRIVATE "-finstrument-functions")
endif()
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

target_include_directories(common_gl PRIVATE ${GL_INC_DIR})
target_include_directories(common_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
target_include_directories(common_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/SDL3-3.2.16/include)
target_include_directories(common_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/imgui-1.91.9b)
target_include_directories(common_gl PRIVATE ${LIB_DIR})
# target_include_directories(common_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/MemDebugger/include)
