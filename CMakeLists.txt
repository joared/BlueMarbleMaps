cmake_minimum_required(VERSION 3.10)
project(BlueMarbleProject)

# Set the compiler to g++
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)
# Set the flags for the compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set the build type (default to "Release" if not specified)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Build options
option(BLUEMARBLE_USE_CIMG_WINDOW "Enable CImg window implementation" ON)
option(BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL "Enable CImg Software drawable implementation" ON)
option(BLUEMARBLE_USE_CIMG_RASTER_IMPL "Enable CImg Raster implementation" ON)

set(LINK_TO_CIMG OFF)
if (BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL OR BLUEMARBLE_USE_CIMG_RASTER_IMPL)
    set(LINK_TO_CIMG ON)
endif()

# Add a library to easy link test cases
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)


