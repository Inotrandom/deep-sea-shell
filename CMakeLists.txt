cmake_minimum_required(VERSION 3.25.1)

add_compile_options(-Wall -Wpedantic -Wextra)

project(DeepSeaShell)
set(CMAKE_CXX_STANDARD 20)

add_executable(
    ${PROJECT_NAME}
    source/dss/example.cpp
    source/dss/runtime.cpp
    source/dss/cli.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
add_library(
    DSS
    source/dss/DSS.cpp 
    source/dss/runtime.cpp 
    source/dss/cli.cpp
)
