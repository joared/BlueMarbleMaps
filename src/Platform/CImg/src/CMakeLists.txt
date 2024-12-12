cmake_minimum_required(VERSION 3.10)

# CMake configuration for the CImg implementation
add_library(CImgImplementation STATIC
    CImgRaster.cpp
    SoftwareDrawableImpl.cpp
    CImgSoftwareRenderer.cpp
)

# Specify include directories for this implementation
# Need to have them public such that BlueMarbleMaps can include them
target_include_directories(CImgImplementation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps
)

# Add any specific compile options or flags, needed if using macro USE_CIMG
target_compile_definitions(CImgImplementation PRIVATE USE_CIMG)

# Link to Cimg
target_link_libraries(CImgImplementation PRIVATE CImgLib)

