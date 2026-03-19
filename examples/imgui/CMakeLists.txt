

add_executable(ExampleImgui main.cpp)

target_link_libraries(ExampleImgui PUBLIC imgui)
if (EMSCRIPTEN)
    target_link_options(ExampleImgui PRIVATE
        -sUSE_GLFW=3
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_definitions(ExampleImgui PRIVATE IMGUI_IMPL_OPENGL_ES3)
    
    # TODO: add this back when it works
    #set_target_properties(ExampleImgui PROPERTIES
    #        SUFFIX ".html")

else()
    target_link_libraries(ExampleImgui PUBLIC Glad)
endif()

#target_link_libraries(ExampleImgui PUBLIC GraphicsRendererGLLib)

#target_link_libraries(ExampleImgui PUBLIC BlueMarbleMapsLib)

