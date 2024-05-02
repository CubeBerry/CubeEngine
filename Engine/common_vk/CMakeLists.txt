# common_vk
cmake_minimum_required(VERSION 3.20)
project(common_vk)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

include_directories(${SRC_DIR} ${INC_DIR} ${LIB_DIR})

FILE(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
FILE(GLOB_RECURSE INCLUDES "${INC_DIR}/*.hpp")

add_library(common_vk STATIC ${SOURCES} ${INCLUDES})

# Vulkan & SDL2 & glm
# find_package(Vulkan REQUIRED)
# target_include_directories(${PROJECT_NAME} ${Vulkan_INCLUDE_DIRS})
# target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

# find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_DIR}/VulkanSDK/1.3.268.0/Include)
target_link_libraries(${PROJECT_NAME} ${LIB_DIR}/VulkanSDK/1.3.268.0/Lib/vulkan-1.lib)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_DIR}/VulkanSDK/1.3.268.0/Include/Lib)
target_link_libraries(${PROJECT_NAME} ${LIB_DIR}/VulkanSDK/1.3.268.0/Lib/SDL2.lib)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_DIR}/VulkanSDK/1.3.268.0/Include/SDL2)