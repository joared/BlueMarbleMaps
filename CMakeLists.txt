cmake_minimum_required(VERSION 3.10)

# Set the compiler to g++
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD 17)

# Set the build type (default to "Release" if not specified)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set the flags for the compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
# add any other flags as needed

# Define the project. Can be used to modify the behaviour of the build.
project(BlueMarbleMaps VERSION 1.0.0 DESCRIPTION "Blue Marble Maps Engine")

# Define the path to the main source file
set(SRC_MAIN main.cpp)

# Define the path to the source directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)

# The source files needed to compile the project executable
set(SOURCES  
    ${SRC_DIR}/Map.cpp
    ${SRC_DIR}/MapConstraints.cpp
    ${SRC_DIR}/Core.cpp
    ${SRC_DIR}/Animation.cpp
    ${SRC_DIR}/EventHandler.cpp
    ${SRC_DIR}/Event.cpp
    ${SRC_DIR}/CImgEventManager.cpp
    ${SRC_DIR}/CImgRaster.cpp
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
    ${SRC_DIR}/Thread.cpp
    ${SRC_DIR}/EngineObject.cpp
    ${SRC_DIR}/LabelOrganizer.cpp
    ${SRC_DIR}/FeatureAnimation.cpp
    ${SRC_DIR}/Effect.cpp
)

# Add the executable
add_executable(BlueMarbleMaps ${SRC_DIR}/${SRC_MAIN} ${SOURCES})
# Add include folder as a private include directory for BlueMarbleEngine
target_include_directories(BlueMarbleMaps
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# Link
target_link_libraries(BlueMarbleMaps PRIVATE CImgLib)

# Example drop shadow
add_executable(ExampleDropShadow ${EXAMPLES_DIR}/example_dropshadow.cpp) #${SOURCES})
target_include_directories(ExampleDropShadow
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ExampleDropShadow PRIVATE CImgLib)

#cmake_policy(SET CMP0072 NEW)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(ExampleGLFW ${EXAMPLES_DIR}/example_glfw.cpp ${EXAMPLES_DIR}/khrplatform.h ${EXAMPLES_DIR}/glad.h ${EXAMPLES_DIR}/glad.c)
target_include_directories(ExampleGLFW
  PRIVATE ${EXAMPLES_DIR}/include
)
target_include_directories(ExampleGLFW
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ExampleGLFW PUBLIC glfw OpenGL::GL ${CMAKE_DL_LIBS})

add_executable(ExampleGLFW2 ${EXAMPLES_DIR}/example_glfw_2.cpp ${EXAMPLES_DIR}/khrplatform.h ${EXAMPLES_DIR}/glad.h ${EXAMPLES_DIR}/glad.c)
target_include_directories(ExampleGLFW2
  PRIVATE ${EXAMPLES_DIR}/include
)
target_include_directories(ExampleGLFW2
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ExampleGLFW2 PUBLIC glfw OpenGL::GL ${CMAKE_DL_LIBS})



add_executable(ExampleTexture ${EXAMPLES_DIR}/example_texture.cpp ${EXAMPLES_DIR}/khrplatform.h ${EXAMPLES_DIR}/glad.h ${EXAMPLES_DIR}/glad.c ${EXAMPLES_DIR}/Texture.h ${EXAMPLES_DIR}/OpenGLTexture.h ${EXAMPLES_DIR}/OpenGLTexture.cpp ${EXAMPLES_DIR}/stb_image.h ${EXAMPLES_DIR}/stb_image.cpp)
# target_include_directories(ExampleTexture
#   PRIVATE ${EXAMPLES_DIR}/include
# )
target_include_directories(ExampleTexture
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${EXAMPLES_DIR}
)
target_link_libraries(ExampleTexture PUBLIC glfw OpenGL::GL ${CMAKE_DL_LIBS} CImgLib)

# Add a library to easy link test cases
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cimg_lib)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/matrix_lib)

# # Check if the BUILD_TESTS flag is set
# set(BUILD_TESTS True)

# if(BUILD_TESTS)
#     # This is needed to install the test executable (dynamic linking)
#     set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

#     set(TEST_MAIN tests.cpp)
#     set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
#     set(TESTS 
#         ${TEST_DIR}/veclib_test.cpp
#         )
#     # Executable for tests, linking to library
#     add_executable(GraphicsEngineTest ${TEST_DIR}/${TEST_MAIN} ${TESTS})
#     target_link_libraries(GraphicsEngineTest GraphicsEngineLib)

#     # Locate GTest
#     find_package(GTest REQUIRED)
#     include_directories(${GTEST_INCLUDE_DIRS})

#     target_link_libraries(GraphicsEngineTest ${GTEST_LIBRARIES} pthread)
#     # Link the test executable with Google Test
#     target_link_libraries(GraphicsEngineTest gtest_main)
    
#     # Add the test executable to CTest
#     include(GoogleTest)
#     gtest_discover_tests(GraphicsEngineTest)
# endif()
