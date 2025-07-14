cmake_minimum_required(VERSION 3.28.3)
project(DeepSeaShell)
set(CMAKE_CXX_STANDARD 20)

add_library(runtime "source/runtime.cpp")
add_library(cli "source/cli.cpp")

add_executable(DeepSeaShell "source/main.cpp")
target_link_libraries(DeepSeaShell PUBLIC runtime)
target_link_libraries(DeepSeaShell PUBLIC cli)