cmake_minimum_required(VERSION 3.10)

# CMake configuration for the CImg implementation
add_library(CImgImplementation STATIC
    RasterImpl.cpp
    SoftwareDrawableImpl.cpp
)

# Link to Cimg. Using public so Bluemarble only needs link to CImgImplementation
target_link_libraries(CImgImplementation PUBLIC CImgLib)

# Specify include directories for this implementation
# Need to have them public such that BlueMarbleMaps can include them
target_include_directories(CImgImplementation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CImgImplementation PRIVATE ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps)

# Add any specific compile options or flags, needed if using macro USE_CIMG
target_compile_definitions(CImgImplementation PRIVATE BLUEMARBLE_USE_CIMG)



