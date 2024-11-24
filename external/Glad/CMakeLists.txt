add_library(Glad STATIC include/glad/glad.h src/glad/glad.c include/KHR/khrplatform.h)

target_include_directories(Renderer
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
