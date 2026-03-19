cmake_minimum_required(VERSION 3.13)

# From https://github.com/nothings/stb

#set(EMSCRIPTEN, True)

#file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
#file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

#add_library(imgui STATIC "${HEADERS}" "${SOURCES}")

#if(EMSCRIPTEN)
#    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
#    target_link_libraries(cmake_vcpkg_imgui_emscripten PRIVATE --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/shell_minimal.html)
#endif()

#target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(imgui PRIVATE glfw)

if(EMSCRIPTEN)

    #find_package(OpenGL REQUIRED)

    add_library(imgui STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
   	${CMAKE_CURRENT_SOURCE_DIR}/examples # for libs/emscripten etc
        #glfw/include
    )

    target_compile_options(imgui PUBLIC
        -sUSE_GLFW=3
        -sUSE_WEBGL2=1
    )

    target_link_options(imgui PUBLIC
        -sUSE_GLFW=3
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
        -sASYNCIFY
    )
    #set_target_properties(imgui PROPERTIES LINK_FLAGS "--no-closure")
    #set_target_properties(imgui PROPERTIES
    #    SUFFIX ".html"
    #)
endif()
if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)

    add_library(imgui STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
        #glfw/include
    )

    target_link_libraries(imgui
        glfw
        #OpenGL::GL
    )
endif()

