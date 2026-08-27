cmake_minimum_required(VERSION 3.10)

#initialize project
set(proj_name matrix)
project(${proj_name})
set(src_files main.cpp)
add_definitions(-w)

#include Eigen
include_directories(${PROJECT_SOURCE_DIR}/../../ext/eigen)

#include glm
include_directories(${root_path}/ext/glm)

#add executable
add_executable(${proj_name} ${src_files})
