# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 26 Dec 2015
#    License: MIT

cmake_minimum_required(VERSION 3.5)

project(inifile-cpp)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "/WX /wd4530")
endif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

add_subdirectory(dep)

include_directories(
    include
)

install(
    FILES "include/inicpp.h"
    DESTINATION include
)

if(${BUILD_TESTS})
    add_subdirectory(test)
endif(${BUILD_TESTS})

if(${BUILD_EXAMPLES})
    add_subdirectory(examples)
endif(${BUILD_EXAMPLES})
