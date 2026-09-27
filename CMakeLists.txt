cmake_minimum_required(VERSION 3.25.0)

project(DeepSeaShell VERSION 0.0.0)

option(DSS_BUILD_TESTS "Whether or not to build Deep Sea Shell unit tests." OFF)

if (DSS_BUILD_TESTS)
	add_executable(
		${PROJECT_NAME} 
		${CMAKE_SOURCE_DIR}/test/test.c
	)

	target_include_directories(
		${PROJECT_NAME}
		PRIVATE
		${CMAKE_SOURCE_DIR}/include
	)
endif()