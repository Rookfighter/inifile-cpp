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


#ifdef THROW_PREVENTED
#define INIF \
  ini::IniFile inif;				\
  bool isOk = inif.tryDecode(str).isOk();	\
  REQUIRE(isOk);
#else
#define INIF \
  ini::IniFile inif;   \
  inif.decode(str);
#endif


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
