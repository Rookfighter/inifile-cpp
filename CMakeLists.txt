# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 26 Dec 2015
#    License: MIT

cmake_minimum_required(VERSION 3.5)

project(IniFileCpp)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_subdirectory("dep")

include_directories(
    "include"
    ${CATCH2_INCLUDE_DIR}
)

set(TEST_SRC "test/main.cpp" "test/test_inifile.cpp")
add_executable(run_tests ${TEST_SRC})

install(
    FILES include/inicpp.h
    DESTINATION include
)
