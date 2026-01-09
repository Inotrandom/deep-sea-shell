cmake_minimum_required(VERSION 3.25.1)

add_compile_options(-Wall -Wpedantic -Wextra)

project(DeepSeaShell)
set(CMAKE_CXX_STANDARD 20)

add_executable(
    ${PROJECT_NAME}
    dss/example.cpp
    dss/runtime.cpp
    dss/cli.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC dss)
add_library(
    DSS
    dss/DSS.cpp 
    dss/runtime.cpp 
    dss/cli.cpp
)
