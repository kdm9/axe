CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(libqes C)

# Append cmake-modules to module path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")


IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version")
	FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version" LIBQES_VERSION)
ELSE()
	# git describe as versioning
	EXECUTE_PROCESS(COMMAND git describe
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE LIBQES_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

#############################
## Setup CMAKE Environment ##
#############################

# Set this before you include libqes as a CMake subproject, so that we know not
# to add cmake to the install rule
IF (LIBQES_AS_SUBMODULE)
    SET(LIBQES_DONT_INSTALL True)
ELSE()
    SET(LIBQES_DONT_INSTALL False)
ENDIF()

OPTION(NO_OPENMP "Disable OpenMP" False)
OPTION(NO_ZLIB "Disable zlib" False)
# Shortcut to enable dev compile options
OPTION(DEV "Enable developer warnings")
IF (DEV)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fsanitize=address -fsanitize=leak -fsanitize=undefined")
    SET(CMAKE_BUILD_TYPE Coverage)
ENDIF()

IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE(STATUS "${CMAKE_BUILD_TYPE} build of ${PROJECT_NAME} version: ${LIBQES_VERSION}")

# Set output directories
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include coverage module IFF out build type is Coverage to avoid bailing out
# with unmet dependencies on Release builds, i.e. other peoples' computers
IF (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    INCLUDE(CodeCoverage)
ENDIF()

# Testing
ENABLE_TESTING()

# Packaging
SET(CPACK_GENERATOR "TGZ;TBZ2")
INCLUDE(CPack)

##########################
## Set Compiler Options ##
##########################


SET(WARN_FLAGS "${WARN_FLAGS} -Wall -Wextra -Wpedantic")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -D_GNU_SOURCE ${WARN_FLAGS}")

SET(CMAKE_C_FLAGS_DEBUG "-ggdb")
SET(CMAKE_C_FLAGS_RELEASE "-O3")


###############################
## Find Packages and Headers ##
###############################

# Header/symbols
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFiles)

CHECK_SYMBOL_EXISTS(vasprintf stdio.h VASPRINTF_FOUND)
CHECK_SYMBOL_EXISTS(asprintf stdio.h ASPRINTF_FOUND)
CHECK_SYMBOL_EXISTS(getline stdio.h GETLINE_FOUND)
CHECK_SYMBOL_EXISTS(strndup string.h STRNDUP_FOUND)

IF (NOT ${NO_ZLIB})
    FIND_PACKAGE(ZLIB 1.2.5 REQUIRED)
    CHECK_LIBRARY_EXISTS(${ZLIB_LIBRARIES} gzbuffer "" GZBUFFER_FOUND)
ELSE()
    SET(ZLIB_FOUND FALSE)
    SET(GZBUFFER_FOUND FALSE)
    SET(ZLIB_C_FLAGS "")
    SET(ZLIB_LIBRARIES "")
    SET(ZLIB_INCLUDE_DIRS "")
    MESSAGE(STATUS "Building without zlib")
ENDIF()

IF (NOT ${NO_OPENMP})
    FIND_PACKAGE(OpenMP)
ELSE()
    SET(OPENMP_FOUND FALSE)
    SET(OpenMP_C_FLAGS "")
    MESSAGE(STATUS "Building without OpenMP")
ENDIF()

# Set dependency flags appropriately
SET(LIBQES_DEPENDS_LIBS
    ${LIBQES_DEPENDS_LIBS}
    ${ZLIB_LIBRARIES})
SET(LIBQES_DEPENDS_INCLUDE_DIRS
    ${LIBQES_DEPENDS_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS})
SET(LIBQES_DEPENDS_CFLAGS
    ${LIBQES_DEPENDS_CFLAGS}
    ${ZLIB_CFLAGS}
    ${OpenMP_C_FLAGS})

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBQES_DEPENDS_CFLAGS}")


#####################
## Set CMake Paths ##
#####################

# Set include dirs
INCLUDE_DIRECTORIES(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/test
    ${CMAKE_CURRENT_SOURCE_DIR}/test/tinytest
    ${LIBQES_DEPENDS_INCLUDE_DIRS}
    )

# Set link dirs
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/lib)

# Traverse to library source and tests
ADD_SUBDIRECTORY(src)
if (NOT LIBQES_AS_SUBMODULE)
    ADD_SUBDIRECTORY(test)
endif()
