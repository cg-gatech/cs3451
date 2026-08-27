cmake_minimum_required(VERSION 3.10)
project(cs3451-computer-graphics)
set (CMAKE_CXX_STANDARD 11)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
add_definitions(-DUSE_STB)
add_subdirectory(assignments)
add_subdirectory(tutorials)
#add_subdirectory(tests)