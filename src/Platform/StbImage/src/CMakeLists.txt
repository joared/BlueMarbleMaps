cmake_minimum_required(VERSION 3.10)

# CMake configuration for the CImg implementation
add_library(StbImageImplementation STATIC "") #Explicit empty list to prevent warning from CMake

# We need some headers from BlueMarbleMaps
target_include_directories(StbImageImplementation PRIVATE ${CMAKE_SOURCE_DIR}/include)
# Link to Cimg. Using public so Bluemarble only needs link to StbImageImplementation
target_link_libraries(StbImageImplementation PUBLIC StbImageLib)

if(NOT BLUEMARBLE_USE_CIMG_RASTER_IMPL)
    message(STATUS "Including Stb image Raster implementation")
    target_sources(StbImageImplementation PRIVATE 
        RasterImpl.cpp
    )
    # Specify include directories for this implementation
    # Need to have them public such that BlueMarbleMaps can include them
    target_include_directories(StbImageImplementation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/RasterImpl)
endif()



