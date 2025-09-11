file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Push up to parent
set(CRS_SOURCES ${SOURCES} PARENT_SCOPE)


