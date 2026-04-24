cmake_minimum_required(VERSION 3.25.1)

project(DeepSeaShell_lib)
option(BUILD_DSS_TESTS OFF)

add_compile_options(-Wextra -Wall -Wpedantic)
set(CMAKE_CXX_STANDARD 20)

if(BUILD_DSS_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()
