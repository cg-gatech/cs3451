cmake_minimum_required(VERSION 3.10)

#initialize project
set(proj_name tutorial_opengl)
project(${proj_name})
add_definitions(-w)

add_subdirectory(glfw-3.3)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

#set paths
if(NOT proj_path)
	set(proj_src_path ${PROJECT_SOURCE_DIR})
endif(NOT proj_path)

if(NOT root_path)
	set(root_path ${proj_src_path}/../../)
	add_definitions(-DROOT_PATH=\"${root_path}\")
endif(NOT root_path)


	
list(APPEND src_files "main.cpp" "glad/glad.cpp")
source_group("proj" FILES ${proj_cpp} ${proj_h})
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
target_link_libraries(${proj_name} ${lib_files} glfw)

include_directories(glad)

set_property(TARGET ${proj_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if(WIN32)
	add_custom_command(TARGET ${proj_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${root_path}/ext/freeglut/bin/x64"
        $<TARGET_FILE_DIR:${proj_name}>)
endif()
