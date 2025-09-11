cmake_minimum_required(VERSION 3.10)

# Define the path to the source directory
set(SRC_DIR ".")

file(GLOB_RECURSE BBM_SOURCES ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps/*.h)

add_subdirectory(Core)
add_subdirectory(CoordinateSystem)
add_subdirectory(Logging)
add_subdirectory(System)
add_subdirectory(Event)
add_subdirectory(Utility)

# The source files needed to compile the project executable
set(SOURCES  
    ${CORE_SOURCES}
    ${CRS_SOURCES}
    ${LOGGING_SOURCES}
    ${SYSTEM_SOURCES}
    ${EVENT_SOURCES}
    ${UTILITY_SOURCES}
    ${BBM_SOURCES}
)

# Add the library with sources set from above add_subdirectory calls
add_library(BlueMarbleMapsLib STATIC ${SOURCES})

# Add include folder as a private include directory for BlueMarbleEngine
target_include_directories(BlueMarbleMapsLib
  PUBLIC ${CMAKE_SOURCE_DIR}/include
)

if(LINK_TO_CIMG)
    message("Linking BlueMarbleMapsLib to CImgImplementation")
    target_link_libraries(BlueMarbleMapsLib PRIVATE CImgImplementation)
endif()

if(LINK_TO_STB_IMAGE)
    message("Linking BlueMarbleMapsLib to StbImageImplementation")
    target_link_libraries(BlueMarbleMapsLib PRIVATE StbImageImplementation)
endif()

if(LINK_TO_GL)
    target_link_libraries(BlueMarbleMapsLib PRIVATE GraphicsRendererGLLib)
endif()


