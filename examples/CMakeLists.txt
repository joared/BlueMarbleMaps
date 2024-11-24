
# Example drop shadow
add_executable(ExampleDropShadow example_dropshadow.cpp)
target_include_directories(ExampleDropShadow
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ExampleDropShadow PRIVATE CImgLib)