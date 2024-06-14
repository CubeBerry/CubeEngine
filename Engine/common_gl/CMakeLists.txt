# common_gl
cmake_minimum_required(VERSION 3.20)

set(GL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(GL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# include_directories(${SRC_DIR} ${INC_DIR} ${LIB_DIR})

FILE(GLOB GL_SOURCES "${GL_SRC_DIR}/*.cpp")
FILE(GLOB GL_INCLUDES "${GL_INC_DIR}/*.hpp")

add_library(common_gl STATIC ${GL_SOURCES} ${GL_INCLUDES})

target_include_directories(common_gl PUBLIC ${GL_INC_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(common_gl PUBLIC ${INC_DIR})
target_include_directories(common_gl PUBLIC ${LIB_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs)
target_include_directories(common_gl PUBLIC ${VK_COMMON_LIB_DIR})

target_link_libraries(common_gl PUBLIC ${LIB_DIR}/glew/glew32.dll)
# target_link_libraries(common_gl PRIVATE ${LIB_DIR}/glew/glew32.lib)
# For MSVC
target_link_libraries(common_gl PUBLIC opengl32)