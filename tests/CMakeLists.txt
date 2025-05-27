function(create_test TEST_NAME)
	add_executable(${TEST_NAME} "${TEST_NAME}.cpp")
	set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 20)
	target_include_directories(${TEST_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
	target_link_libraries(${TEST_NAME} ${PROJECT_NAME})
	add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()

create_test("1")
