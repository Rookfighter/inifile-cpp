/*
 * test_inifile.cpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#include <catch.hpp>
#include "inicpp.h"

#include <sstream>
#include <cmath>

TEST_CASE("decode ini file", "IniFile")
{
    std::istringstream ss(("[Foo]\nbar=hello world\n[Test]"));
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "hello world");
    REQUIRE(inif["Test"].size() == 0);
}

TEST_CASE("decode empty file", "IniFile")
{
    std::istringstream ss("");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 0);
}



TEST_CASE("fail to decode file with section not closed", "IniFile")
{
  std::istringstream ss(("[Foo]\nbar=hello world\n[Test\nfoo=never reached"));
  ini::IniFile inif;
  
  CHECK_THROWS_AS(inif.decode(ss),  std::logic_error);
}

TEST_CASE("fail to load unclosed section", "IniFile")
{
    ini::IniFile inif;
    REQUIRE_THROWS_AS(inif.decode("[Foo\nbar=bla"),  std::logic_error);
}



TEST_CASE("fail to decode file with empty section name", "IniFile")
{
  std::istringstream ss(("[Foo]\nbar=hello world\n[]\nfoo=never reached"));
  ini::IniFile inif;
  
  CHECK_THROWS_AS(inif.decode(ss),  std::logic_error);
}

TEST_CASE("fail to decode file with text after section", "IniFile")
{
  std::istringstream ss(("[Foo]\nbar=hello world\n[Test]superfluous\nfoo=never reached"));
  ini::IniFile inif;
  
  CHECK_THROWS_AS(inif.decode(ss),  std::logic_error);
}

TEST_CASE("fail to decode file with field without section", "IniFile")
{
  std::istringstream ss(("; comment only\nbar=hello world\n[Test]\nfoo=say goodby"));
  ini::IniFile inif;
  
  CHECK_THROWS_AS(inif.decode(ss),  std::logic_error);
}

TEST_CASE("fail to load field without section", "IniFile")
{
    ini::IniFile inif;
    REQUIRE_THROWS_AS(inif.decode("bar=bla"),  std::logic_error);
}


TEST_CASE("fail to decode file with field without separator", "IniFile")
{
  std::istringstream ss(("[Foo]\nbar no_separator\n[Test]\nfoo=never reached"));
  ini::IniFile inif;
  
  CHECK_THROWS_AS(inif.decode(ss),  std::logic_error);
}

TEST_CASE("fail to load field without equal", "IniFile")
{
    ini::IniFile inif;
    REQUIRE_THROWS_AS(inif.decode("[Foo]\nbar"),  std::logic_error);
}


TEST_CASE("decode file with comment only", "IniFile")
{
    std::istringstream ss("# this is a comment");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 0);
}


TEST_CASE("decode file with empty section", "IniFile")
{
    std::istringstream ss("[Foo]");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 0);
}

TEST_CASE("parse empty field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "");
}

// TBD: this seems to be very crude to me 
TEST_CASE("parse section with duplicate field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello\nbar=World");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "World");
}

/***************************************************
 * get as type
 ***************************************************/

TEST_CASE("parse field as double", "IniFile")
{
    std::istringstream ss("[Foo]"
			  "\nbar0=-0" "\nbar1=1.2" "\nbar2=1" "\nbar3=-2.4"
			  "\nbar4=-nan" "\nbar5=-inF" "\nbar6=infinity"
			  "\nbar7=-2.5e4");
    ini::IniFile inif(ss); 

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
    REQUIRE(sec["bar0"].as<double>() == 0.0);
    REQUIRE(std::signbit(sec["bar0"].as<double>()));
    REQUIRE(sec["bar1"].as<double>() == 1.2);
    REQUIRE(sec["bar2"].as<double>() == 1.0);
    REQUIRE(sec["bar3"].as<double>() == -2.4);
    REQUIRE(std::isnan  (sec["bar4"].as<double>()));
    REQUIRE(std::signbit(sec["bar4"].as<double>()));
    REQUIRE(sec["bar5"].as<double>() == -INFINITY);
    REQUIRE(sec["bar6"].as<double>() == +INFINITY);
    REQUIRE(sec["bar7"].as<double>() == -2.5e4);
}

TEST_CASE("fail to parse as double", "IniFile")
{
   std::istringstream ss("[Foo]"
			  "\nbar1=bla" "\nbar1=-2.5e4x");
    ini::IniFile inif(ss); 

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
    REQUIRE_THROWS_AS(sec["bar1"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned int>(), std::invalid_argument);
}

TEST_CASE("parse field as (unsigned) int, fail if negative unsigned", "IniFile")
{
    std::istringstream ss("[Foo]" "\nbar0=0" "\nbar1=1" "\nbar2=-42"
			  "\nbar80=00" "\nbar81=02" "\nbar82=-07"
			  "\nbarG0=-0x0" "\nbarG1=0xfF" "\nbarG2=-0x80");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
    REQUIRE(sec["bar0"].as<         int>() == 0);
    REQUIRE(sec["bar0"].as<unsigned int>() == 0);
    REQUIRE(sec["bar1"].as<         int>() == 1);
    REQUIRE(sec["bar1"].as<unsigned int>() == 1);
    REQUIRE(sec["bar2"].as<         int>() == -42);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned int>(), std::invalid_argument);

    REQUIRE(sec["bar80"].as<         int>() == 0);
    REQUIRE(sec["bar80"].as<unsigned int>() == 0);
    REQUIRE(sec["bar81"].as<         int>() == 2);
    REQUIRE(sec["bar81"].as<unsigned int>() == 2);
    REQUIRE(sec["bar82"].as<int>() == -7);
    REQUIRE_THROWS_AS(sec["bar82"].as<unsigned int>(), std::invalid_argument);

    
    REQUIRE(sec["barG0"].as<int>() == 0);
    REQUIRE_THROWS_AS(sec["baG0"].as<unsigned int>(), std::invalid_argument);
    REQUIRE(sec["barG1"].as<         int>() == 255);
    REQUIRE(sec["barG1"].as<unsigned int>() == 255);

    REQUIRE(sec["barG2"].as<         int>() == -128);
    // TBC: baG2 shall be NULL
    REQUIRE_THROWS_AS(sec["baG2"].as<unsigned int>(), std::invalid_argument);
}


TEST_CASE("fail to parse as (unsigned) int", "IniFile")
{
    std::istringstream ss("[Foo]"
			  "\nbar1=bla" "\nbar2=" "\nbar3=2x" "\nbar4=+"
			  "\nbar82=08" "\nbarG0=0x" "\nbarG1=0xg");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
    REQUIRE_THROWS_AS(sec["bar1"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar1"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned  int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar3"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar3"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar4"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar4"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar82"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar82"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG0"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG0"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG1"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG1"].as<unsigned int>(), std::invalid_argument);


    
}



TEST_CASE("parse field as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=true\nbar2=false\nbar3=tRuE");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].as<bool>());
    REQUIRE_FALSE(inif["Foo"]["bar2"].as<bool>());
    REQUIRE(inif["Foo"]["bar3"].as<bool>());
}

TEST_CASE("parse field with custom field sep", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1:true\nbar2:false\nbar3:tRuE");
    ini::IniFile inif(ss, ':');

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].as<bool>());
    REQUIRE_FALSE(inif["Foo"]["bar2"].as<bool>());
    REQUIRE(inif["Foo"]["bar3"].as<bool>());
}

TEST_CASE("parse file with comment", "IniFile")
{
    std::istringstream ss("[Foo]\n# this is a test\nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("parse with custom comment char", "IniFile")
{
    std::istringstream ss("[Foo]\n$ this is a test\nbar=bla");
    ini::IniFile inif(ss, '=', '$');

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("save with bool fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=true\nbar2=false\n");
}

TEST_CASE("save with int fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = 1;
    inif["Foo"]["bar2"] = -2;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=1\nbar2=-2\n");
}

// TBD: infinite and NaN
TEST_CASE("save with double fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = 1.2;
    inif["Foo"]["bar2"] = -2.4;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=1.2\nbar2=-2.4\n");
}

TEST_CASE("save with custom field sep", "IniFile")
{
    ini::IniFile inif(':', '#');
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1:true\nbar2:false\n");
}



/***************************************************
 * get as type
 ***************************************************/
// TBD: 

TEST_CASE("fail to parse as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE_THROWS(inif["Foo"]["bar"].as<bool>());
}



/***************************************************
 * Treatment of spaces 
 ***************************************************/



TEST_CASE("spaces are not taken into account in field names", "IniFile")
{
    std::istringstream ss(("[Foo]\n  \t  bar  \t  =hello world"));
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"].find("bar") != inif["Foo"].end());
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "hello world");
}

TEST_CASE("spaces are not taken into account in field values", "IniFile")
{
    std::istringstream ss(("[Foo]\nbar=  \t  hello world  \t  "));
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "hello world");
}

TEST_CASE("spaces are not taken into account in sections", "IniFile")
{
    std::istringstream ss("  \t  [Foo]  \t  \nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.find("Foo") != inif.end());
}
