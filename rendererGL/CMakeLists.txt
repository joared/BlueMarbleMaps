cmake_minimum_required(VERSION 3.10)

#project(GraphicsRendererGL)

find_package(GLFW3 REQUIRED)
find_package(GLAD REQUIRED)
find_package(GLM REQUIRED)

add_executable(GraphicsRendererGL ${GL_RENDERER_DIR}/renderer.cpp) #${SOURCES})
target_include_directories(GraphicsRendererGL 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

message(STATUS "Looking for GLFW")

include_directories(${GLFW3_INCLUDE_DIR})
target_link_libraries(GraphicsRendererGL ${GLFW3_LIBRARY})

message(STATUS "Looking for GLAD")

include_directories(${GLAD_INCLUDE_DIR})
target_link_libraries(GraphicsRendererGL ${GLAD_LIBRARIES})

message(STATUS "Looking for GLM")

include_directories(${GLM_INCLUDE_DIR})

