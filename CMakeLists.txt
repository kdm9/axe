cmake_minimum_required(VERSION 2.8)
project(trieBCD)

set(VERSION "0.0a1-git")
# Cmake options
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
enable_testing()

# CC options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -DTRIEBCD_VERSION=\\\"${VERSION}\\\"")
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

# Set CFLAGS
SET(EXTRA_WARNING_FLAGS "-fstack-protector-all -Wstack-protector -Wfloat-equal -Wundef -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wredundant-decls -Wchar-subscripts -Wcomment -Wformat=2 -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wbad-function-cast -Wswitch-enum -Winit-self -Wmissing-field-initializers -Wdeclaration-after-statement -Wold-style-definition -Waddress -Wmissing-noreturn -Wstrict-overflow=1 -Wnormalized=id -Woverride-init -Wextra -Warray-bounds -Wlogical-op")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -fopenmp -Wall ${KMLIB_DEPENDS_CFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DKMLIB_VERSION=\\\"${KMLIB_VERSION}\\\" ${EXTRA_WARNING_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-ggdb")
SET(CMAKE_C_FLAGS_RELEASE "-O3")
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/datrie ${CMAKE_SOURCE_DIR}/src/kmlib/src)
link_directories(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(tests)
add_subdirectory(src)
