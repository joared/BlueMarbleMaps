cmake_minimum_required(VERSION 3.10)

if(BLUEMARBLE_USE_CIMG_WINDOW)
    message("Using CImg window main")
    add_executable(BlueMarbleMapsCImg main_cimg.cpp)
    target_link_libraries(BlueMarbleMapsCImg CImgLib)
    target_link_libraries(BlueMarbleMapsCImg BlueMarbleMapsLib)
else
    message("Using GLFW window main")
    add_executable(BlueMarbleMapsGLFW main_glfw.cpp)
    target_link_libraries(BlueMarbleMapsCImg CImgLib)
    target_link_libraries(BlueMarbleMapsCImg BlueMarbleMapsLib)
endif()

