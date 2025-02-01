cmake_minimum_required(VERSION 3.10)

# Define the path to the source directory
set(SRC_DIR ".")

file(GLOB_RECURSE BBM_SOURCES ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps/*.h)

# The source files needed to compile the project executable
set(SOURCES  
    ${SRC_DIR}/Map.cpp
    ${SRC_DIR}/MapConstraints.cpp
    ${SRC_DIR}/Core.cpp
    ${SRC_DIR}/Animation.cpp
    ${SRC_DIR}/Event/EventHandler.cpp
    ${SRC_DIR}/Raster.cpp
    ${SRC_DIR}/Layer.cpp
    ${SRC_DIR}/DataSet.cpp
    ${SRC_DIR}/SoftwareDrawable.cpp
    ${SRC_DIR}/Geometry.cpp
    ${SRC_DIR}/Feature.cpp
    ${SRC_DIR}/UpdateInterfaces.cpp
    ${SRC_DIR}/File.cpp
    ${SRC_DIR}/PresentationObject.cpp
    ${SRC_DIR}/Visualizer.cpp
    ${SRC_DIR}/Algorithm.cpp
    ${SRC_DIR}/EngineObject.cpp
    ${SRC_DIR}/LabelOrganizer.cpp
    ${SRC_DIR}/FeatureAnimation.cpp
    ${SRC_DIR}/Effect.cpp
    ${SRC_DIR}/MapControl.cpp
    ${SRC_DIR}/OpenGLDrawable.cpp
    ${SRC_DIR}/CoordinateSystem/Crs.cpp
    ${SRC_DIR}/CoordinateSystem/GeodeticDatum.cpp
    ${SRC_DIR}/CoordinateSystem/Projection.cpp
    ${BBM_SOURCES}
)

# Add the executable
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


