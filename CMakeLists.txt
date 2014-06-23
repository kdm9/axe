cmake_minimum_required(VERSION 2.8)
project(trieBCD)

# Cmake options
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
enable_testing()

# CC options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_SOURCE_DIR}/src /home/kevin/prog/kmlib/src)
link_directories(${CMAKE_BINARY_DIR}/lib /home/kevin/prog/kmlib/build/lib)

add_subdirectory(tests)
add_subdirectory(src)
