cmake_minimum_required(VERSION 3.25.1)

project(DeepSeaShell_test)

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/test/test.cpp
)
