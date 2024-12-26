cmake_minimum_required(VERSION 3.10)

add_library(StbImageLib STATIC include/stb_image.h stb_image.cpp include/stb_image_resize.h stb_image_resize.cpp)

target_include_directories(StbImageLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

