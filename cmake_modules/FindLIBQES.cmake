# - Find libqes
# Find the native libqes includes and library.
# Once done this will define
#
#  LIBQES_INCLUDE_DIRS   - where to find qes.h, etc.
#  LIBQES_LIBRARIES      - List of libraries when using libqes.
#  LIBQES_FOUND          - True if libqes found.
#
#  LIBQES_VERSION_STRING - The version of libqes found (x.y.z)
#  LIBQES_VERSION_MAJOR  - The major version of libqes
#  LIBQES_VERSION_MINOR  - The minor version of libqes
#  LIBQES_VERSION_PATCH  - The patch version of libqes
#  LIBQES_VERSION_PREREL - The pre-release version of libqes
#  LIBQES_VERSION_GIT    - The git version of libqes
#
# An includer may set LIBQES_ROOT to a libqes installation root to tell
# this module where to look.

#=============================================================================
# Copyright 2014 Kevin Murray. Adapted from FindZLIB.cmake
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

set(_LIBQES_SEARCHES)

# Search LIBQES_ROOT first if it is set.
if(LIBQES_ROOT)
  set(_LIBQES_SEARCH_ROOT PATHS ${LIBQES_ROOT} NO_DEFAULT_PATH)
  list(APPEND _LIBQES_SEARCHES _LIBQES_SEARCH_ROOT)
endif()

# Normal search.
set(_LIBQES_SEARCH_NORMAL
  PATHS "$ENV{PROGRAMFILES}/libqes"
  )
list(APPEND _LIBQES_SEARCHES _LIBQES_SEARCH_NORMAL)

# Try each search configuration.
foreach(search ${_LIBQES_SEARCHES})
  find_path(LIBQES_INCLUDE_DIR NAMES qes.h ${${search}} PATH_SUFFIXES include)
  find_library(LIBQES_LIBRARY  NAMES qes ${${search}} PATH_SUFFIXES lib)
endforeach()

mark_as_advanced(LIBQES_LIBRARY LIBQES_INCLUDE_DIR)
# Handle version. Again, flogged from zlib
if(LIBQES_INCLUDE_DIR AND EXISTS "${LIBQES_INCLUDE_DIR}/qes_config.h")
    file(STRINGS "${LIBQES_INCLUDE_DIR}/qes_config.h" LIBQES_H REGEX "^#define LIBQES_VERSION \"[^\"]*\"")

    string(REGEX REPLACE "^.*LIBQES_VERSION \"([0-9]+).*$" "\\1" LIBQES_VERSION_MAJOR "${LIBQES_H}")
    string(REGEX REPLACE "^.*LIBQES_VERSION \"[0-9]+\\.([0-9]+).*$" "\\1" LIBQES_VERSION_MINOR  "${LIBQES_H}")
    string(REGEX REPLACE "^.*LIBQES_VERSION \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" LIBQES_VERSION_PATCH "${LIBQES_H}")
    set(LIBQES_VERSION_STRING "${LIBQES_VERSION_MAJOR}.${LIBQES_VERSION_MINOR}.${LIBQES_VERSION_PATCH}")

    # only append a EXTRA version if it exists:
    set(LIBQES_VERSION_EXTRA "")
    if( "${LIBQES_H}" MATCHES "^.*LIBQES_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+(.+)\\+git.*$")
        set(LIBQES_VERSION_PREREL "${CMAKE_MATCH_1}")
    endif()
    if( "${LIBQES_H}" MATCHES "^.*LIBQES_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+.*\\+git\\.(.+)$")
        set(LIBQES_VERSION_git "${CMAKE_MATCH_1}")
    endif()
    set(LIBQES_VERSION_STRING "${LIBQES_VERSION_STRING}${LIBQES_VERSION_PREREL}")
endif()

# handle the QUIETLY and REQUIRED arguments and set LIBQES_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBQES REQUIRED_VARS LIBQES_LIBRARY LIBQES_INCLUDE_DIR
                                       VERSION_VAR LIBQES_VERSION_STRING)

if(LIBQES_FOUND)
    set(LIBQES_INCLUDE_DIRS ${LIBQES_INCLUDE_DIR})
    set(LIBQES_LIBRARIES ${LIBQES_LIBRARY})
endif()

