# Copyright 2014-2015 Kevin Murray <spam@kdmurray.id.au>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Axe library (libaxe.a)
FILE(GLOB DATRIE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/datrie/*.c)
FILE(GLOB GSL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/gsl/*.c)
SET(AXELIB_SRCS ${DATRIE_SRCS} axe.c)

IF (NOT GSL_FOUND)
    MESSAGE(STATUS "Using bundled GSL sources")
    SET(AXELIB_SRCS ${AXELIB_SRCS} ${GSL_SRCS})
ENDIF()

ADD_LIBRARY(axelib STATIC ${AXELIB_SRCS})
TARGET_LINK_LIBRARIES(axelib qes_static ${AXE_DEP_LIBS})
SET_TARGET_PROPERTIES(axelib PROPERTIES OUTPUT_NAME axe)

# Executable
ADD_EXECUTABLE(axe-demux main.c)
TARGET_LINK_LIBRARIES(axe-demux ${AXE_DEPENDS_LIBS} axelib)
INSTALL(TARGETS axe-demux DESTINATION "bin")
