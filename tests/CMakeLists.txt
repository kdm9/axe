INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/tinytest)

ADD_EXECUTABLE(test_axe test.c ${CMAKE_CURRENT_SOURCE_DIR}/tinytest/tinytest.c test_libaxe.c)
TARGET_LINK_LIBRARIES(test_axe ${ZLIB_LIBRARIES} axelib)

# Copy test files over to bin dir & make output
ADD_CUSTOM_COMMAND(TARGET test_axe
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
		   ${CMAKE_CURRENT_SOURCE_DIR}/data
		   ${CMAKE_BINARY_DIR}/data)
ADD_CUSTOM_COMMAND(TARGET test_axe
                   COMMAND ${CMAKE_COMMAND} -E make_directory
		   ${CMAKE_BINARY_DIR}/out)

ADD_TEST(NAME "Unit_tests" COMMAND test_axe)
ADD_TEST(NAME "Single_end_mode" COMMAND ${CMAKE_BINARY_DIR}/bin/axe
	 -f ${CMAKE_BINARY_DIR}/data/pare.fq.gz
	 -F ${CMAKE_BINARY_DIR}/out/pare_se
	 -u ${CMAKE_BINARY_DIR}/out/pare_se_unbarcoded.fq
	 -b ${CMAKE_BINARY_DIR}/data/pare.barcodes)
ADD_TEST(NAME "valgrind_Single_end_mode" COMMAND
	 valgrind --leak-check=full
	 ${CMAKE_BINARY_DIR}/bin/axe
	 -f ${CMAKE_BINARY_DIR}/data/pare.fq.gz
	 -F ${CMAKE_BINARY_DIR}/out/pare_se
	 -u ${CMAKE_BINARY_DIR}/out/pare_se_unbarcoded.fq
	 -b ${CMAKE_BINARY_DIR}/data/pare.barcodes)
