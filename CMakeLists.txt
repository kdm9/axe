cmake_minimum_required(VERSION 2.8)
project(axe C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

# Cmake options
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
enable_testing()

IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF()

# git versioning
INCLUDE(GetGitRevisionDescription)
get_git_head_revision(rev hash)
git_get_exact_tag(tag)

IF("${tag}")
	set(VERSION "${tag}")
ELSE()
	git_describe(gitdesc "--abbrev=0")
	if("${gitdesc}")
		set(VERSION "${gitdesc}")
	ELSE()
		set(VERSION "dirty")
	ENDIF()
ENDIF()
IF (NOT "${hash}" STREQUAL "")
	STRING(SUBSTRING ${hash} 0 7 hash)
	set(VERSION "${VERSION}+git.${hash}")
ENDIF()

MESSAGE(STATUS "${CMAKE_BUILD_TYPE} build of axe version: ${VERSION}")

# CC options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -DAXE_VERSION=\\\"${VERSION}\\\"")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O4 -Werror")

###############################
## Find Packages and Headers ##
###############################

FIND_PACKAGE(ZLIB 1.2.5 REQUIRED)
FIND_PACKAGE(LIBQES 0.0.1 REQUIRED)
SET(AXE_DEPENDS_LIBS
    ${AXE_DEPENDS_LIBS} ${ZLIB_LIBRARIES} ${LIBQES_LIBRARIES})
MESSAGE(STATUS "Libs are ${AXE_DEPENDS_LIBS}")
SET(AXE_DEPENDS_INCLUDE_DIRS
    ${AXE_DEPENDS_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${LIBQES_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${AXE_DEPENDS_INCLUDE_DIRS})

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
SET(AXEWRN "${AXEWRN} -Wstrict-overflow=1 -Wextra -Warray-bounds -Wall")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${AXEWRN}")
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/datrie)
link_directories(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(docs)
add_subdirectory(tests)
add_subdirectory(src)
