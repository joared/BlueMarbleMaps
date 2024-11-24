add_library(Glad STATIC include/glad/glad.h src/glad.c include/KHR/khrplatform.h)

target_include_directories(Glad
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
