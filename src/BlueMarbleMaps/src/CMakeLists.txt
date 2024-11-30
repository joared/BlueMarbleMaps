cmake_minimum_required(VERSION 3.10)

# Define the path to the main source file
set(SRC_MAIN main.cpp)

# Define the path to the source directory
set(SRC_DIR ".")

file(GLOB_RECURSE BBM_SOURCES ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps/*.h)

# The source files needed to compile the project executable
set(SOURCES  
    ${SRC_DIR}/Map.cpp
    ${SRC_DIR}/MapConstraints.cpp
    ${SRC_DIR}/Core.cpp
    ${SRC_DIR}/Animation.cpp
    ${SRC_DIR}/EventHandler.cpp
    ${SRC_DIR}/Event.cpp
    ${SRC_DIR}/Drawable.cpp
    ${SRC_DIR}/Layer.cpp
    ${SRC_DIR}/DataSet.cpp
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
    ${BBM_SOURCES}
)

# Add the executable
add_executable(BlueMarbleMaps ${SRC_DIR}/${SRC_MAIN} ${SOURCES})
add_library(BlueMarbleMapsLib ${SOURCES})
# Add include folder as a private include directory for BlueMarbleEngine
target_include_directories(BlueMarbleMaps
  PRIVATE ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps
)
target_include_directories(BlueMarbleMapsLib
  PRIVATE ${CMAKE_SOURCE_DIR}/include/BlueMarbleMaps
)
option(USE_CIMG "Enable CImg-based implementation" ON)
if(USE_CIMG)
    #add_subdirectory(implementations/cimg)
    message("Linking to CImgLib")
    #target_link_libraries(BlueMarbleMaps CImgLib)
    message("Linking to CImgImplementation")
    #target_link_libraries(BlueMarbleMaps CImgImplementation)
endif()


