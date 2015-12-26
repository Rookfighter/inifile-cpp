cmake_minimum_required(VERSION 2.0)
project(IniFileCpp)
set(LIB_NAME "inifilecpp")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(SRC "src/IniFile.cpp")
include_directories("src")

add_library(${LIB_NAME} ${SRC})