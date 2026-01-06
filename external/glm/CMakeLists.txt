add_library(glm INTERFACE)

target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(glm::glm ALIAS glm)
