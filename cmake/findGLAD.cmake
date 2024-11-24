
SET(_glad_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes"
"C:/Program Files (x86)/glad"
"C:/dependencies/includes")
# check environment variable
SET(_glad_ENV_ROOT_DIR "$ENV{GLAD_ROOT_DIR}")
IF(NOT GLAD_ROOT_DIR AND _glad_ENV_ROOT_DIR)
	SET(GLAD_ROOT_DIR "${_glad_ENV_ROOT_DIR}")
ENDIF(NOT GLAD_ROOT_DIR AND _glad_ENV_ROOT_DIR)
# put user specified location at beginning of search
IF(GLAD_ROOT_DIR)
	SET(_glad_HEADER_SEARCH_DIRS "${GLAD_ROOT_DIR}"
	"${GLAD_ROOT_DIR}/include"
	${_glad_HEADER_SEARCH_DIRS})
ENDIF(GLAD_ROOT_DIR)
# locate header
FIND_PATH(GLAD_INCLUDE_DIR "glad/glad.c" "glad/glad.h"
PATHS ${_glad_HEADER_SEARCH_DIRS})
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLAD DEFAULT_MSG
GLAD_INCLUDE_DIR)
IF(GLAD_FOUND)
	SET(GLAD_INCLUDE_DIRS "${GLAD_INCLUDE_DIR}")
	MESSAGE(STATUS "GLAD_INCLUDE_DIR = ${GLAD_INCLUDE_DIR}")
ENDIF(GLAD_FOUND)