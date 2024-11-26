cmake_minimum_required(VERSION 3.10)

#project(GraphicsRendererGL)

find_package(GLFW3 REQUIRED)

#find_package(GLAD REQUIRED)
#find_package(GLM REQUIRED)

add_executable(GraphicsRendererGL renderer.cpp) #${SOURCES})
target_include_directories(GraphicsRendererGL 
  PRIVATE include
)

message(STATUS "Looking for GLFW")

target_include_directories(GraphicsRendererGL PRIVATE ${GLFW3_INCLUDE_DIR})
target_link_libraries(GraphicsRendererGL ${GLFW3_LIBRARY})

message(STATUS "Looking for GLAD")

target_link_libraries(GraphicsRendererGL Glad)

message(STATUS "Looking for GLM")

target_link_libraries(GraphicsRendererGL glm)

