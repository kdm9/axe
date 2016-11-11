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

# =============================================================================
# Copyright 2014 Kevin Murray. Adapted from FindZLIB.cmake
# Licensed under the 3-clause BSD license
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
# REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

    string(REGEX REPLACE "^.*LIBQES_VERSION \"[Vv]?([0-9]+).*$" "\\1" LIBQES_VERSION_MAJOR "${LIBQES_H}")
    string(REGEX REPLACE "^.*LIBQES_VERSION \"[Vv]?[0-9]+\\.([0-9]+).*$" "\\1" LIBQES_VERSION_MINOR  "${LIBQES_H}")
    string(REGEX REPLACE "^.*LIBQES_VERSION \"[Vv]?[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" LIBQES_VERSION_PATCH "${LIBQES_H}")
    set(LIBQES_VERSION_STRING "${LIBQES_VERSION_MAJOR}.${LIBQES_VERSION_MINOR}.${LIBQES_VERSION_PATCH}")

    # only append a EXTRA version if it exists:
    set(LIBQES_VERSION_EXTRA "")
    if( "${LIBQES_H}" MATCHES "^.*LIBQES_VERSION \"[Vv]?[0-9]+\\.[0-9]+\\.[0-9]+(.+)\\+git.*$")
        set(LIBQES_VERSION_PREREL "${CMAKE_MATCH_1}")
    endif()
    if( "${LIBQES_H}" MATCHES "^.*LIBQES_VERSION \"[Vv]?[0-9]+\\.[0-9]+\\.[0-9]+.*\\+git\\.(.+)$")
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

