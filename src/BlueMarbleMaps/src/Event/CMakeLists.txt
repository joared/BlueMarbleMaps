file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Push up to parent
set(EVENT_SOURCES ${SOURCES} PARENT_SCOPE)


