cmake_minimum_required(VERSION 3.13)

#project(GraphicsRendererGL)

#find_package(GLFW3 REQUIRED)
#find_package(GLAD REQUIRED)
#find_package(GLM REQUIRED)

file(GLOB_RECURSE RENDERER_SOURCES 
    "*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Application/WindowGL.cpp"
    "${CMAKE_SOURCE_DIR}/external/stb_image/stb_image.h"
    "${CMAKE_SOURCE_DIR}/external/stb_image/stb_image.cpp"
    "${CMAKE_SOURCE_DIR}/include/Application/WindowGL.h"
    "${CMAKE_SOURCE_DIR}/include/Platform/OpenGL/*.h"
)
file(GLOB_RECURSE RENDERER_SOURCES2 
    "Shader.cpp"
    "Rect.cpp"
    "Primitive.cpp"
    "Texture.cpp"
    "VAO.cpp"
    "VBO.cpp"
    "IBO.cpp"
    "Batch.cpp"
    "Algorithms.cpp"
    "${CMAKE_SOURCE_DIR}/include/Platform/OpenGL/*.h"
)
file(GLOB SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.vert"
)

if (NOT EMSCRIPTEN)
find_package(GLFW3 REQUIRED)
add_executable(GraphicsRendererGL ${RENDERER_SOURCES} ${SHADER_SOURCES}) 

target_include_directories(GraphicsRendererGL PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")

target_include_directories(GraphicsRendererGL PRIVATE "${CMAKE_SOURCE_DIR}/include/Platform/OpenGL")

message(STATUS "Looking for GLFW")

target_include_directories(GraphicsRendererGL PRIVATE ${GLFW3_INCLUDE_DIR})
target_link_libraries(GraphicsRendererGL ${GLFW3_LIBRARY})

message(STATUS "Looking for GLAD")

target_link_libraries(GraphicsRendererGL Glad)

message(STATUS "Looking for GLM")

target_link_libraries(GraphicsRendererGL glm)

message(STATUS "looking for stb_image")
target_link_libraries(GraphicsRendererGL StbImageLib)

#set_source_files_properties(${SHADER_SOURCES} PROPERTIES LANGUAGE GLSL)

source_group("shaders" FILES ${SHADER_SOURCES})

add_custom_command(
        TARGET GraphicsRendererGL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_PROPERTY:GraphicsRendererGL,SOURCE_DIR>/Shaders"
            "$<TARGET_PROPERTY:GraphicsRendererGL,BINARY_DIR>/Shaders"
    )

endif()

add_library(GraphicsRendererGLLib ${RENDERER_SOURCES2} ${SHADER_SOURCER})

target_include_directories(GraphicsRendererGLLib PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(GraphicsRendererGLLib PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")
target_include_directories(GraphicsRendererGLLib PUBLIC "${CMAKE_SOURCE_DIR}/include/Platform/OpenGL")

message(STATUS "Looking for GLFW")

if (NOT EMSCRIPTEN)
    find_package(GLFW3 REQUIRED)
    target_include_directories(GraphicsRendererGLLib PUBLIC ${GLFW3_INCLUDE_DIR})
    target_link_libraries(GraphicsRendererGLLib PUBLIC ${GLFW3_LIBRARY})
    target_link_libraries(GraphicsRendererGLLib PUBLIC Glad)
endif()

target_link_libraries(GraphicsRendererGLLib PUBLIC glm)
target_link_libraries(GraphicsRendererGLLib PUBLIC StbImageLib)

#set_source_files_properties(${SHADER_SOURCES} PROPERTIES LANGUAGE GLSL)

source_group("shaders" FILES ${SHADER_SOURCES})

add_custom_command(
        TARGET GraphicsRendererGLLib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_PROPERTY:GraphicsRendererGLLib,SOURCE_DIR>/Shaders"
            "$<TARGET_PROPERTY:GraphicsRendererGLLib,BINARY_DIR>/Shaders"
    )
