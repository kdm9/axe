CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(axe C)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

# Cmake options
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
ENABLE_TESTING()

IF (NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE Release)
ENDIF()

IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version")
	FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version" AXE_VERSION)
ELSE()
	# git describe as versioning
	EXECUTE_PROCESS(COMMAND git describe
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE AXE_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

MESSAGE(STATUS "${CMAKE_BUILD_TYPE} build of axe version: ${AXE_VERSION}")

###############################
## Find Packages and Headers ##
###############################

# all from CMakeLists.txt in libqes

# Header/symbols
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFiles)

CHECK_SYMBOL_EXISTS(getline stdio.h GETLINE_FOUND)
CHECK_SYMBOL_EXISTS(strndup string.h STRNDUP_FOUND)
CHECK_INCLUDE_FILES(malloc.h MALLOC_H_FOUND)
IF(MALLOC_H_FOUND)
    CHECK_SYMBOL_EXISTS(memalign malloc.h MEMALIGN_FOUND)
ELSE()
    SET(MEMALIGN_FOUND False)
ENDIF()
CHECK_SYMBOL_EXISTS(getpagesize unistd.h GETPAGESIZE_FOUND)

FIND_PACKAGE(ZLIB 1.2.1 REQUIRED)
CHECK_LIBRARY_EXISTS(${ZLIB_LIBRARIES} gzbuffer "" GZBUFFER_FOUND)

# Set dependency flags appropriately
SET(LIBQES_DEPENDS_LIBS
    ${LIBQES_DEPENDS_LIBS}
    ${ZLIB_LIBRARIES})
SET(LIBQES_DEPENDS_INCLUDE_DIRS
    ${LIBQES_DEPENDS_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS})
SET(LIBQES_DEPENDS_CFLAGS
    ${LIBQES_DEPENDS_CFLAGS}
    ${ZLIB_CFLAGS})

##########################
## Set Compiler Options ##
##########################

IF (CMAKE_COMPILER_IS_GNUCC)
        SET(AXEWRN "${AXEWRN} -Woverride-init -Wnormalized=id -Wlogical-op")
        EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                        OUTPUT_VARIABLE GCC_VERSION)
        IF (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
                SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
        ENDIF()
ENDIF()

# Set CFLAGS
SET(AXEWRN "${AXEWRN} -fstack-protector-all -Wstack-protector -Wfloat-equal")
SET(AXEWRN "${AXEWRN} -Wundef -Wpointer-arith -Wstrict-prototypes")
SET(AXEWRN "${AXEWRN} -Wmissing-prototypes -Wwrite-strings -Wredundant-decls")
SET(AXEWRN "${AXEWRN} -Wchar-subscripts -Wcomment -Wformat=2 -Wwrite-strings")
SET(AXEWRN "${AXEWRN} -Wmissing-declarations -Wredundant-decls -Wnested-externs")
SET(AXEWRN "${AXEWRN} -Wbad-function-cast -Wswitch-enum -Winit-self")
SET(AXEWRN "${AXEWRN} -Wmissing-field-initializers -Wdeclaration-after-statement")
SET(AXEWRN "${AXEWRN} -Wold-style-definition -Waddress -Wmissing-noreturn ")
SET(AXEWRN "${AXEWRN} -Wstrict-overflow=1 -Wextra -Warray-bounds -Wall -D_FORTIFY_SOURCE=2")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${AXEWRN} ${LIBQES_DEPENDS_CFLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src/datrie
                    ${CMAKE_SOURCE_DIR}/src/libqes/src
                    ${CMAKE_SOURCE_DIR}/src/gsl
                    ${LIBQES_DEPENDS_INCLUDE_DIRS})
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/lib)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/axe_config.h.in
               ${CMAKE_BINARY_DIR}/axe_config.h)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/libqes/src/qes_config.h.in
               ${CMAKE_BINARY_DIR}/qes_config.h)

ADD_SUBDIRECTORY(docs)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(src)
