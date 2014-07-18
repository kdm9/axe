cmake_minimum_required(VERSION 2.8)
project(axe C)

set(VERSION "0.0a1-git")
# Cmake options
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
enable_testing()

# CC options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -DAXE_VERSION=\\\"${VERSION}\\\"")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Werror")

###############################
## Find Packages and Headers ##
###############################

FIND_PACKAGE(ZLIB 1.2.5 REQUIRED)
SET(KMLIB_DEPENDS_LIBS
    ${KMLIB_DEPENDS_LIBS} ${ZLIB_LIBRARIES})
SET(KMLIB_DEPENDS_INCLUDE_DIRS
    ${KMLIB_DEPENDS_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
SET(KMLIB_DEPENDS_CFLAGS
    "${KMLIB_DEPENDS_CFLAGS} -DHAVE_ZLIB")

##########################
## Set Compiler Options ##
##########################

# When STATIC is TRUE, then cmake only looks for static libraries
IF(LINUX)
    IF(STATIC)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    ENDIF(STATIC)
ENDIF(LINUX)

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
SET(AXEWRN "${AXEWRN} -Wstrict-overflow=1 -Wextra -Warray-bounds")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -fopenmp -Wall ${KMLIB_DEPENDS_CFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DKMLIB_VERSION=\\\"${KMLIB_VERSION}\\\" ${AXEWRN}")
SET(CMAKE_C_FLAGS_DEBUG "-ggdb -O0")
SET(CMAKE_C_FLAGS_RELEASE "-O3")
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/datrie ${CMAKE_SOURCE_DIR}/src/kmlib/src)
link_directories(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(tests)
add_subdirectory(src)
