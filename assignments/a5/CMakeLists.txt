cmake_minimum_required(VERSION 3.10)

#initialize project
set(proj_name a5)
project(${proj_name})
add_definitions(-w)

#set paths
if(NOT proj_path)
	set(proj_src_path ${PROJECT_SOURCE_DIR})
endif(NOT proj_path)

if(NOT root_path)
	set(root_path ${proj_src_path}/../../)
	add_definitions(-DROOT_PATH=\"${root_path}\")
endif(NOT root_path)

#include Eigen
include_directories(${root_path}/ext/eigen)

#include header and source files in /src
file(GLOB_RECURSE src_cpp ${root_path}/src/*.cpp)
file(GLOB_RECURSE src_h ${root_path}/src/*.h)
file(GLOB_RECURSE src_frag ${root_path}/src/*.frag)
file(GLOB_RECURSE src_vert ${root_path}/src/*.vert)
file(GLOB_RECURSE src_glsl ${root_path}/src/*.glsl)
list(APPEND src_files ${src_cpp} ${src_h} ${src_frag} ${src_vert} ${src_glsl})
source_group("src" FILES ${src_cpp} ${src_h} ${src_frag} ${src_vert} ${src_glsl})
include_directories(${root_path}/src)
	
#include header and source files in /proj/a*
file(GLOB_RECURSE proj_cpp ${proj_src_path}/*.cpp)
file(GLOB_RECURSE proj_h ${proj_src_path}/*.h)
file(GLOB_RECURSE proj_frag ${proj_src_path}/*.frag)
file(GLOB_RECURSE proj_vert ${proj_src_path}/*.vert)
file(GLOB_RECURSE proj_glsl ${proj_src_path}/*.glsl)
list(APPEND src_files ${proj_cpp} ${proj_h} ${proj_frag} ${proj_vert} ${proj_glsl})
source_group("proj" FILES ${proj_cpp} ${proj_h} ${proj_frag} ${proj_vert} ${proj_glsl})
include_directories(${proj_path}/src)

#include glm
include_directories(${root_path}/ext/glm)

#include freeglut and glew
if(WIN32)
	set(freeglut_src_path ${root_path}/ext/freeglut/include)
	set(freeglut_lib_path ${root_path}/ext/freeglut/lib/x64)
	include_directories(${freeglut_src_path})

	set(glut_libs debug ${freeglut_lib_path}/freeglutd.lib optimized ${freeglut_lib_path}/freeglut.lib)
	message(STATUS ${glut_libs})
	list(APPEND lib_files ${glut_libs})
		
elseif(APPLE)  #freeglut and glew are installed on macos by "brew install freeglut glew"
	find_package(GLUT REQUIRED)
	if (GLUT_FOUND)
    		include_directories(${GLUT_INCLUDE_DIRS})
    		link_libraries(${GLUT_LIBRARIES})
	endif()

	find_package(OpenGL REQUIRED)
    	include_directories(${OpenGL_INCLUDE_DIRS})
    	link_directories(${OpenGL_LIBRARY_DIRS})
    	add_definitions(${OpenGL_DEFINITIONS})

	list(APPEND lib_files ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

elseif(UNIX) #freeglut and glew are installed on linux by "sudo apt-get install freeglut3-dev"
	set(GCC_COVERAGE_COMPILE_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} -lGL -lglut -lGLU -ldl")
	set(GCC_COVERAGE_LINK_FLAGS "${GCC_COVERAGE_LINK_FLAGS} -lGL -lglut -lGLU -ldl")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
	set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
endif(WIN32)

#include tiny_obj_loader
file(GLOB_RECURSE tiny_obj_cpp ${root_path}/ext/tiny_obj_loader/*.cpp)
file(GLOB_RECURSE tiny_obj_h ${root_path}/ext/tiny_obj_loader/*.h)
list(APPEND src_files ${tiny_obj_cpp} ${tiny_obj_h})
source_group("tiny_obj_loader" FILES ${tiny_obj_cpp} ${tiny_obj_h})
include_directories(${root_path}/ext/tiny_obj_loader)

#include stb
file(GLOB_RECURSE stb_cpp ${root_path}/ext/stb/*.cpp)
file(GLOB_RECURSE stb_h ${root_path}/ext/stb/*.h)
list(APPEND src_files ${stb_cpp} ${stb_h})
source_group("stb" FILES ${stb_cpp} ${stb_h})
include_directories(${root_path}/ext/stb)

#include tiny_gltf
file(GLOB_RECURSE tiny_gltf_cpp ${root_path}/ext/tiny_gltf/*.cpp)
file(GLOB_RECURSE tiny_gltf_h ${root_path}/ext/tiny_gltf/*.h ${root_path}/ext/tiny_gltf/*.hpp)
list(APPEND src_files ${tiny_gltf_cpp} ${tiny_gltf_h})
source_group("tiny_gltf" FILES ${tiny_gltf_cpp} ${tiny_gltf_h})
include_directories(${root_path}/ext/tiny_gltf)

#set compiling flags
set(CMAKE_CXX_STANDARD 11)	#c++11
if(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")	#c++11
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")	#turn off sign-compare warning
endif(UNIX)
if(WIN32)
	add_definitions(-D_DISABLE_EXTENDED_ALIGNED_STORAGE)	#fix compiling issue for VS2017
endif(WIN32)

#add executable
add_executable(${proj_name} ${src_files})
target_link_libraries(${proj_name} ${lib_files})

set_property(TARGET ${proj_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if(WIN32)
	add_custom_command(TARGET ${proj_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${root_path}/ext/freeglut/bin/x64"
        $<TARGET_FILE_DIR:${proj_name}>)
endif()
