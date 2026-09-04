cmake_minimum_required(VERSION 3.10)

add_library(gif-h STATIC include/gif.h dummy.cpp)

target_include_directories(gif-h
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

