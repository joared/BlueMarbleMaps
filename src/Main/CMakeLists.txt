cmake_minimum_required(VERSION 3.10)

if(BLUEMARBLE_USE_CIMG_WINDOW)
    message("Linking to Main to CImgImplementation")
    add_executable(BlueMarbleMapsCImg main_cimg.cpp)
    target_link_libraries(BlueMarbleMapsCImg CImgLib)
    target_link_libraries(BlueMarbleMapsCImg BlueMarbleMapsLib)
endif()

