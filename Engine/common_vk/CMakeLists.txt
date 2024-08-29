# common_vk
cmake_minimum_required(VERSION 3.20)

set(VK_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(VK_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# include_directories(${SRC_DIR} ${INC_DIR} ${LIB_DIR})

FILE(GLOB VK_SOURCES "${VK_SRC_DIR}/*.cpp")
FILE(GLOB VK_INCLUDES "${VK_INC_DIR}/*.hpp")

add_library(common_vk STATIC ${VK_SOURCES} ${VK_INCLUDES})

target_include_directories(common_vk PRIVATE ${VK_INC_DIR})

target_include_directories(common_vk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
target_include_directories(common_vk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/SDL2-2.30.6/include)
target_include_directories(common_vk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libs/imgui)
target_include_directories(common_vk PUBLIC ${LIB_DIR}/VulkanSDK/1.3.268.0/Include)