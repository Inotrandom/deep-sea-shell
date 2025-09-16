cmake_minimum_required(VERSION 3.28.3)

project(DeepSeaShell)
set(CMAKE_CXX_STANDARD 20)

add_executable(
    ${PROJECT_NAME}
    source/example.cpp
    source/runtime.cpp
    source/cli.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
add_library(DSS source/DSS.cpp)