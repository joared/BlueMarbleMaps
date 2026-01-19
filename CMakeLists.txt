cmake_minimum_required(VERSION 3.10)
project(BlueMarbleProject)

# Set the compiler to g++
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)
# Set the flags for the compiler (probably do this from the IDE instead, otherwise its overriden)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

# Put runtime (executables) into bin/<config>
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# Put libraries into lib/<config>
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/lib/RelWithDebInfo)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)

# Put archives (static libs, etc.) into lib/<config>
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/lib/RelWithDebInfo)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set the build type (default to "Release" if not specified)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Build options
option(BLUEMARBLE_USE_CIMG_WINDOW "Enable CImg window implementation" OFF)
option(BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL "Enable CImg Software drawable implementation" ON)
option(BLUEMARBLE_USE_CIMG_RASTER_IMPL "Enable CImg Raster implementation" OFF)

set(LINK_TO_CIMG OFF)
if (BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL OR BLUEMARBLE_USE_CIMG_RASTER_IMPL)
    set(LINK_TO_CIMG ON)
endif()

set(LINK_TO_STB_IMAGE OFF)
if (NOT BLUEMARBLE_USE_CIMG_RASTER_IMPL)
    set(LINK_TO_STB_IMAGE ON)
endif()

set(LINK_TO_GL ON)

message("Finding GDAL")
find_package(GDAL REQUIRED)

# Add a library to easy link test cases
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)


