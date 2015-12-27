cmake_minimum_required(VERSION 2.0)
project(IniFileCpp)
set(LIB_NAME "inifilecpp")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

set(SRC "src/IniFile.cpp")
include_directories("src")

add_library(${LIB_NAME} ${SRC})

include_directories("deps/Catch")
set(TEST_NAME "Test${PROJECT_NAME}")
set(TEST_SRC "test/TestIniFile.cpp")
add_executable(${TEST_NAME} ${TEST_SRC})
target_link_libraries(${TEST_NAME} ${LIB_NAME})