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

# all are now handled by libqes

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
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${AXEWRN}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src/datrie
                    ${CMAKE_SOURCE_DIR}/src/gsl)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/lib)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/libqes/src)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_SUBDIRECTORY(docs)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(src/libqes)
