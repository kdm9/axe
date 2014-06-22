#CFLAGS
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinytest)

add_executable(test_tbd test.c ${CMAKE_CURRENT_SOURCE_DIR}/tinytest/tinytest.c test_libtbd.c)
target_link_libraries(test_tbd z km tbd)

add_test(NAME test COMMAND test_tbd)
