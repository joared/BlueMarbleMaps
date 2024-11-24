add_library(glm STATIC include/glad/glad.h src/glad/glad.c include/KHR/khrplatform.h)

target_include_directories(glm
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
