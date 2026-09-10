if(WIN32)
    message("Using Socket_Win32")
    set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Socket_Win32.cpp)
else()
    message("Using Socket_Posix")
    set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Socket_Posix.cpp)
endif()

# Push up to parent
set(NETWORKING_SOURCES ${SOURCES} PARENT_SCOPE)


