/*
 * main.cpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <filesystem>
#include <iostream>
#include <string.h>



#define TESTFILE(NAME) ((std::filesystem::current_path() / \
			 ".." / "test" / "iniFiles" / NAME).string())




//#define SSTREAM_PREVENTED
//#define   THROW_PREVENTED

#include "test_inifile.cpp"


#define SSTREAM_PREVENTED
//#define   THROW_PREVENTED

#include "test_inifile.cpp"


#define SSTREAM_PREVENTED
#define   THROW_PREVENTED

#include "test_inifile.cpp"


//#define SSTREAM_PREVENTED
#define   THROW_PREVENTED

#include "test_inifile.cpp"

