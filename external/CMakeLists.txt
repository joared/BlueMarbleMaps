if (NOT EMSCRIPTEN)
    
    add_subdirectory(Glad)
    add_subdirectory(cimg_lib)
    
    add_subdirectory(refl-cpp)
endif()

# TODO: only this works wit EMSCRIPTEN
add_subdirectory(glm)
add_subdirectory(stb_image)
add_subdirectory(refl-cpp)
add_subdirectory(imgui)
