CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(axe C)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

# Cmake options
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
ENABLE_TESTING()

IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF()

# git versioning
INCLUDE(GetGitRevisionDescription)
get_git_head_revision(rev hash)
git_get_exact_tag(tag)

IF("${tag}")
	SET(VERSION "${tag}")
ELSE()
	git_describe(gitdesc "--abbrev=0")
	#special test, tags starting with 0 fail if you just to "if (x)"
	if(NOT "${gitdesc}" MATCHES "^-")
		SET(VERSION "${gitdesc}")
	ELSE()
		SET(VERSION "dirty")
	ENDIF()
ENDIF()
IF (NOT "${hash}" STREQUAL "")
	STRING(SUBSTRING ${hash} 0 7 hash)
	set(VERSION "${VERSION}+git.${hash}")
ENDIF()

MESSAGE(STATUS "${CMAKE_BUILD_TYPE} build of axe version: ${VERSION}")


###############################
## Find Packages and Headers ##
###############################

FIND_PACKAGE(ZLIB 1.2.5 REQUIRED)
FIND_PACKAGE(GSL 1.5 REQUIRED)
SET(AXE_DEPENDS_LIBS
    ${AXE_DEPENDS_LIBS} ${ZLIB_LIBRARIES} ${GSL_LIBRARIES})
MESSAGE(STATUS "Libs are ${AXE_DEPENDS_LIBS}")
SET(AXE_DEPENDS_INCLUDE_DIRS
    ${AXE_DEPENDS_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS})
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

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -DAXE_VERSION=\\\"${VERSION}\\\"")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${AXEWRN}")
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/datrie)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/lib)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/libqes/src)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
SET(LIBQES_VERSION "bundled_in_axe")
SET(NO_OPENMP 1)

ADD_SUBDIRECTORY(docs)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(src)
