if (NOT EMSCRIPTEN)
    add_subdirectory(Glad)
endif()
add_subdirectory(cimg_lib)
add_subdirectory(glm)
add_subdirectory(stb_image)
add_subdirectory(refl-cpp)

# Special handling to make imgui find glfw
find_package(GLFW3 REQUIRED)
add_library(glfw UNKNOWN IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW3_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_INCLUDE_DIR}"
)
add_subdirectory(imgui)
add_subdirectory(gif-h)
