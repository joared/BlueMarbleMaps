cmake_minimum_required(VERSION 3.10)

# CMake configuration for the CImg implementation
add_library(CImgImplementation STATIC "") #Explicit empty list to prevent warning from CMake

# We need some headers from BlueMarbleMaps
target_include_directories(CImgImplementation PRIVATE ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps)
# Link to Cimg. Using public so Bluemarble only needs link to CImgImplementation
target_link_libraries(CImgImplementation PUBLIC CImgLib)

if(BLUEMARBLE_USE_CIMG_RASTER_IMPL)
    message(STATUS "Including CImg Raster implementation")
    target_sources(CImgImplementation PRIVATE 
        RasterImpl.cpp
    )
    # Specify include directories for this implementation
    # Need to have them public such that BlueMarbleMaps can include them
    target_include_directories(CImgImplementation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/RasterImpl)
    target_compile_definitions(CImgImplementation PUBLIC BLUEMARBLE_USE_CIMG_RASTER_IMPL)
endif()

if(BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL)
    message(STATUS "Including CImg Software drawable implementation")
    target_sources(CImgImplementation PRIVATE 
        SoftwareDrawableImpl.cpp
    )
    target_include_directories(CImgImplementation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/SoftwareDrawableImpl)
    target_compile_definitions(CImgImplementation PUBLIC BLUEMARBLE_USE_CIMG_SOFTWARE_DRAWABLE_IMPL)
endif()



