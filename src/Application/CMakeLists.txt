cmake_minimum_required(VERSION 3.10)

find_package(GLFW3 REQUIRED)

file(GLOB_RECURSE APPLICATION_SOURCES 
    "${CMAKE_SOURCE_DIR}/include/Application/*.h"
    "${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps/*.h"
)

if(BLUEMARBLE_USE_CIMG_WINDOW)
    message("Using CImg window main")
    add_executable(Application main_cimg.cpp)
    target_link_libraries(Application BlueMarbleMapsLib)
    target_link_libraries(Application CImgLib)
else()
    message("Using GLFW window main")
    add_executable(Application main_glfw.cpp WindowGL.cpp)
    target_link_libraries(Application glm)
    target_link_libraries(Application BlueMarbleMapsLib)
    target_link_libraries(Application CImgLib)
    find_package(GLFW3 REQUIRED)
    target_include_directories(Application PRIVATE ${GLFW3_INCLUDE_DIR})
    target_link_libraries(Application ${GLFW3_LIBRARY})
    target_link_libraries(Application Glad)
endif()
target_include_directories(Application PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")

# Post build: copy OpenGL shaders
add_custom_command(TARGET Application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Platform/OpenGL/Shaders
        $<TARGET_FILE_DIR:Application>/Shaders
)
