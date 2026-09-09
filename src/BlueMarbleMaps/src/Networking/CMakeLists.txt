if(WIN32)
	set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Socket_Win32.cpp)
else()
	set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Socket_Posix.cpp)
endif()

# Push up to parent
set(NETWORKING_SOURCES ${SOURCES} PARENT_SCOPE)


