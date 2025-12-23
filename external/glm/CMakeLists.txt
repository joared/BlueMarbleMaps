file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

add_library(glm STATIC ${SOURCES})

target_include_directories(glm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
#target_include_directories(glm PUBLIC include)

#set_target_properties(glm PROPERTIES LINKER_LANGUAGE CXX)


if(CMAKE_DL_LIBS)
    # Link against the dynamic loading library if it is available
    target_link_libraries(glm PRIVATE ${CMAKE_DL_LIBS})
endif()
