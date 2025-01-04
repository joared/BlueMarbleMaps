cmake_minimum_required(VERSION 3.10)

find_package(GLFW3 REQUIRED)

file(GLOB_RECURSE APPLICATION_SOURCES 
    "*.cpp"
    "${CMAKE_SOURCE_DIR}/include/Application/*.h"
    "${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps/*.h"
)

if(BLUEMARBLE_USE_CIMG_WINDOW)
    message("Using CImg window main")
    add_executable(Application main_cimg.cpp)
    target_link_libraries(Application CImgLib)
    target_link_libraries(Application BlueMarbleMapsLib)
else()
    message("Using GLFW window main")
    add_executable(Application ${APPLICATION_SOURCES})
    target_link_libraries(Application CImgLib)
    target_link_libraries(Application BlueMarbleMapsLib)
    find_package(GLFW3 REQUIRED)
    target_include_directories(Application PRIVATE ${GLFW3_INCLUDE_DIR})
    target_link_libraries(Application ${GLFW3_LIBRARY})
    target_link_libraries(Application Glad)
endif()
target_include_directories(Application PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")

