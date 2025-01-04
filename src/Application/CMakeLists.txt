cmake_minimum_required(VERSION 3.10)

find_package(GLFW3 REQUIRED)

file(GLOB_RECURSE APPLICATION_SOURCES 
    "*.cpp"
    "${CMAKE_SOURCE_DIR}/include/Application/*.h"
    "${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps/*.h"
)

add_executable(Application ${APPLICATION_SOURCES}) 
target_include_directories(Application PRIVATE"${CMAKE_SOURCE_DIR}/include/Common")
target_include_directories(Application PRIVATE "${CMAKE_SOURCE_DIR}/include/Application")
target_link_libraries(Application PRIVATE BlueMarbleMapsLib)

message(STATUS "Looking for GLFW")

target_include_directories(Application PRIVATE ${GLFW3_INCLUDE_DIR})
target_link_libraries(Application ${GLFW3_LIBRARY})

target_include_directories(Application PRIVATE BLUE)

