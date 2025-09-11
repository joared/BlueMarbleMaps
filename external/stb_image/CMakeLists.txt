cmake_minimum_required(VERSION 3.10)

# From https://github.com/nothings/stb

add_library(StbImageLib STATIC 
	    include/stb_image.h stb_image.cpp 
	    include/stb_image_resize2.h stb_image_resize2.cpp
	    include/stb_image_write.h stb_image_write.cpp)

target_include_directories(StbImageLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

