cmake_minimum_required(VERSION 3.10)

#project(GraphicsRendererGL)

find_package(GLFW3 REQUIRED)
#find_package(GLAD REQUIRED)
#find_package(GLM REQUIRED)

file(GLOB_RECURSE RENDERER_SOURCES 
    "*.cpp"
    "${CMAKE_SOURCE_DIR}/include/RendererGL/*.h"
)

add_executable(GraphicsRendererGL ${RENDERER_SOURCES}) 

target_include_directories(GraphicsRendererGL PRIVATE "${CMAKE_SOURCE_DIR}/include/RendererGL/")

message(STATUS "Looking for GLFW")

target_include_directories(GraphicsRendererGL PRIVATE ${GLFW3_INCLUDE_DIR})
target_link_libraries(GraphicsRendererGL ${GLFW3_LIBRARY})

message(STATUS "Looking for GLAD")

target_link_libraries(GraphicsRendererGL Glad)

message(STATUS "Looking for GLM")

target_link_libraries(GraphicsRendererGL glm)

