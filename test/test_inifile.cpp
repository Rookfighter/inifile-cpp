/*
 * test_inifile.cpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#include <catch.hpp>
// TBD: tests whether throws prevented or not. 
//#define THROW_PREVENTED
#include "inicpp.h"


#include <string.h>


#ifdef THROW_PREVENTED
#define INIF \
  ini::IniFile inif;				\
  bool isOk = inif.tryDecode(ss).isOk();	\
  REQUIRE(isOk);				
#else
#define INIF \
    ini::IniFile inif(ss);
#endif

// TBD: tests missing:
// open files for encoding/decoding where this does not work:
// cannot open:
// - not exist
// - dir
// - not readable/writable
// No idea how to test bad bit. 

TEST_CASE("decode ini file", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello world\n[Test]\n\n");
    INIF

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"]["bar"].toString() == "hello world");
    REQUIRE(inif["Test"].size() == 0);
}

TEST_CASE("decode empty file", "IniFile")
{
    std::istringstream ss("");
    INIF
      // TBD: clarify whether it is ok to set an exception here:
      // stream has fail bit. 
      REQUIRE(inif.size() == 0);
}



TEST_CASE("fail to decode file with section not closed", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello world\n[Test\nfoo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(ss);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_NOT_CLOSED);
    REQUIRE(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(ss),  std::logic_error);
#endif
}


TEST_CASE("fail to decode file with empty section name", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello world\n[]\nfoo=never reached");
   ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(ss);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_NAME_EMPTY);
    REQUIRE(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(ss),  std::logic_error);
#endif
}

TEST_CASE("fail to decode file with text after section", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello world\n[Test]superfluous\nfoo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(ss);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_TEXT_AFTER);
    REQUIRE(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(ss),  std::logic_error);
#endif
}

TEST_CASE("fail to decode file with field without section", "IniFile")
{
    std::istringstream ss("# comment\nbar=hello world\n[Test]\nfoo=say goodby");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(ss);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::FIELD_WITHOUT_SECTION);
    REQUIRE(deResult.getLineNumber() == 2);
#else
    REQUIRE_THROWS_AS(inif.decode(ss),  std::logic_error);
#endif
}


TEST_CASE("fail to decode file with illegal line", "IniFile")
{
    std::istringstream ss("[Foo]\nbar no_separator\n[Test]\nfoo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(ss);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::ILLEGAL_LINE);
    REQUIRE(deResult.getLineNumber() == 2);
#else
    REQUIRE_THROWS_AS(inif.decode(ss),  std::logic_error);
#endif
}

TEST_CASE("fail to decode file with field without custom separator", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=no_separator\n[Test]\nfoo=never reached");
   ini::IniFile inif(':', '#');
#ifdef THROW_PREVENTED
     ini::IniFile::DecEncResult deResult = inif.tryDecode(ss);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::ILLEGAL_LINE);
    REQUIRE(deResult.getLineNumber() == 2);
#else
    REQUIRE_THROWS_AS(inif.decode(ss),  std::logic_error);
#endif
}


TEST_CASE("decode file with comment only", "IniFile")
{
    std::istringstream ss("# this is a comment");
    INIF
    //ini::IniFile inif(ss);

    REQUIRE(inif.size() == 0);
}


TEST_CASE("decode file with empty section", "IniFile")
{
    std::istringstream ss("[Foo]");
    INIF
    //ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 0);
}

TEST_CASE("parse empty field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=");
    INIF
    //ini::IniFile inif(ss);

    CHECK(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].toString() == "");
}

// TBD: this seems to be very crude to me 
TEST_CASE("parse section with duplicate field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello\nbar=World");
    INIF
    //ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].toString() == "World");
}

/***************************************************
 * get as type
 ***************************************************/


TEST_CASE("parse field as c-string", "IniFile")
{
    std::istringstream ss("[Foo]" "\nbar=blaC");
     
    INIF
    //ini::IniFile inif(ss); 
    const char* nvrOcc = "never occurs";
    REQUIRE(inif.size() == 1);
#ifndef THROW_PREVENTED
    REQUIRE(strcmp(inif["Foo"]["bar"].as<const char*>(), "blaC") == 0);
#endif
    REQUIRE(strcmp(inif["Foo"]["bar"].orDefault(nvrOcc), "blaC") == 0);
    REQUIRE(!inif["Foo"]["bar"].failedLastOutConversion());
}

TEST_CASE("parse field as std::string", "IniFile")
{
    std::istringstream ss("[Foo]" "\nbar=blaS");
     
    INIF
    //ini::IniFile inif(ss); 
    const char* nvrOcc = "never occurs";

    REQUIRE(inif.size() == 1);
#ifndef THROW_PREVENTED
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == std::string("blaS"));
#endif
    REQUIRE(inif["Foo"]["bar"].orDefault(nvrOcc) == std::string("blaS"));
    REQUIRE(!inif["Foo"]["bar"].failedLastOutConversion());
}

TEST_CASE("parse field as double", "IniFile")
{
    std::istringstream ss("[Foo]"
			  "\nbar0=-0" "\nbar1=1.2" "\nbar2=1" "\nbar3=-2.4"
			  "\nbarNF4=-nan" "\nbarNF5=-inF" "\nbarNF6=infinity"
			  "\nbar7=-2.5e4"
			  "\nbarL1=" 
"1797693134862315708145274237317043567980705675258449965989174768031572607800"
"2853876058955863276687817154045895351438246423432132688946418276846754670353"
"7516986049910576551282076245490090389328944075868508455133942304583236903222"
"9481658085593321233482747978262041447231687381771809192998812504040261841248"
			 "58368.0" //  max 
			  //std::to_string(std::numeric_limits<double>::max())// +
			  //"\nbarL2=" +
			  //std::to_string(std::numeric_limits<double>::min())
			 "\nbarL2=" 
"1797693134862315708145274237317043567980705675258449965989174768031572607800"
"2853876058955863276687817154045895351438246423432132688946418276846754670353"
"7516986049910576551282076245490090389328944075868508455133942304583236903222"
"9481658085593321233482747978262041447231687381771809192998812504040261841248"
			 "58368.01" // with 68.0 this is max 
			 "\nbarL3=" 
"1797693134862315708145274237317043567980705675258449965989174768031572607800"
"2853876058955863276687817154045895351438246423432132688946418276846754670353"
"7516986049910576551282076245490090389328944075868508455133942304583236903222"
"9481658085593321233482747978262041447231687381771809192998812504040261841248"
			 "5836800.01" // with 68.0 this is max 
			  );
    INIF
    //ini::IniFile inif(ss); 

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];


#ifndef THROW_PREVENTED   
    REQUIRE(sec["bar0"].as<double>()    == 0.0);
    REQUIRE(std::signbit(sec["bar0"].as<double>()));
    REQUIRE(sec["bar1"].as<double>()   == 1.2);
    REQUIRE(sec["bar2"].as<double>()   == 1.0);
    REQUIRE(sec["bar3"].as<double>()   == -2.4);
    REQUIRE(std::isnan  (sec["barNF4"].as<double>()));
    REQUIRE(std::signbit(sec["barNF4"].as<double>()));
    REQUIRE(sec["barNF5"].as<double>()   == -INFINITY);
    REQUIRE(sec["barNF6"].as<double>()    == +INFINITY);
    REQUIRE(sec["bar7"].as<double>()   == -2.5e4);
    REQUIRE(sec["barL1"].as<double>()   == std::numeric_limits<double>::max());
    REQUIRE(sec["barL2"].as<double>()   == std::numeric_limits<double>::max());
    REQUIRE(sec["barL3"].as<double>()   == INFINITY);
#endif

    
    REQUIRE(sec["bar0"].orDefault(-1.0) == 0.0);
    REQUIRE(std::signbit(sec["bar0"].orDefault(-1.0)));
    REQUIRE(sec["bar1"].orDefault(0.0) == 1.2);
    REQUIRE(sec["bar2"].orDefault(0.0) == 1.0);
    REQUIRE(sec["bar3"].orDefault(0.0) == -2.4);
    REQUIRE(std::isnan  (sec["barNF4"].orDefault(-0.1)));
    REQUIRE(std::signbit(sec["barNF4"].orDefault(-0.1)));
    REQUIRE(sec["barNF5"].orDefault(1.0) == -INFINITY);
    REQUIRE(sec["barNF6"].orDefault(-1.0) == +INFINITY);
    REQUIRE(sec["bar7"].orDefault(0.0) == -2.5e4);
    REQUIRE(sec["barL1"].orDefault(0.0) == std::numeric_limits<double>::max());
    REQUIRE(sec["barL2"].orDefault(0.0) == std::numeric_limits<double>::max());
    REQUIRE(sec["barL3"].orDefault(0.0) == INFINITY);
}

TEST_CASE("fail to parse field as double", "IniFile")
{
   std::istringstream ss("[Foo]"
			 "\nbar1=bla" "\nbar2=-2.5e4x" "\nbar3="
			 );
    INIF
    //ini::IniFile inif(ss); 

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED
    REQUIRE_THROWS_AS(sec["bar1"].as<double>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<double>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar3"].as<double>(), std::invalid_argument);
#endif

    REQUIRE(sec["bar1"].orDefault(0.1) == 0.1);
    REQUIRE(sec["bar1"].failedLastOutConversion());
    REQUIRE(sec["bar2"].orDefault(0.2) == 0.2);
    REQUIRE(sec["bar2"].failedLastOutConversion());
    REQUIRE(sec["bar3"].orDefault(0.3) == 0.3);
    REQUIRE(sec["bar3"].failedLastOutConversion());
}


TEST_CASE("parse field as float", "IniFile")
{
    std::istringstream ss("[Foo]"
			  "\nbar0=-0" "\nbar1=1.2" "\nbar2=1" "\nbar3=-2.4"
			  "\nbarNF4=-nan" "\nbarNF5=-inF" "\nbarNF6=infinity"
			  "\nbar7=-2.5e4"
			  "\nbarL1="
			  "340282346638528859811704183484516925440.0" // max
			  //std::numeric_limits<float>::max()
			  "\nbarL2=" 
			  "340282346638528859811704183484516925440.0001" // 
			  "\nbarL3=" 
			  "340282346638528859811704183484516925440000.1" // 
			  );
    INIF
    //ini::IniFile inif(ss); 

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED  
    REQUIRE(sec["bar0"].as<float>()     == 0.0f);
    REQUIRE(std::signbit(sec["bar0"].as<float>()));
    REQUIRE(sec["bar1"].as<float>()     == 1.2f);
    REQUIRE(sec["bar2"].as<float>()     == 1.0f);
    REQUIRE(sec["bar3"].as<float>()     == -2.4f);
    // TBD: document that the sign bit is not carried, or change that 
    REQUIRE(std::isnan  (sec["barNF4"].as<float>()));
    REQUIRE(std::signbit(sec["barNF4"].as<float>()));
    REQUIRE(sec["barNF5"].as<float>()     == -INFINITY);
    REQUIRE(sec["barNF6"].as<float>()     == +INFINITY);
    REQUIRE(sec["bar7"].as<float>()    == -2.5e4f);
    REQUIRE(sec["barL1"].as<float>()    == std::numeric_limits<float>::max());

    // TBC
    // REQUIRE(sec["barL2"].as<float>() == std::numeric_limits<float>::max());
    REQUIRE(sec["barL3"].as<float>()    == INFINITY);
#endif



    
    REQUIRE(sec["bar0"].orDefault(-.1f) == 0.0f);
    REQUIRE(std::signbit(sec["bar0"].orDefault(-1.1f)));
    REQUIRE(sec["bar1"].orDefault(-.1f) == 1.2f);
    REQUIRE(sec["bar2"].orDefault(-.1f) == 1.0f);
    REQUIRE(sec["bar3"].orDefault(.1f)  == -2.4f);
    // TBD: document that the sign bit is not carried, or change that 
    REQUIRE(std::isnan  (sec["barNF4"].orDefault(.1f)));
    REQUIRE(std::signbit(sec["barNF4"].orDefault(.1f)));
    REQUIRE(sec["barNF5"].orDefault(+.1f) == -INFINITY);
    REQUIRE(sec["barNF6"].orDefault(-.1f) == +INFINITY);
    REQUIRE(sec["bar7"].orDefault(.1f) == -2.5e4f);
    REQUIRE(sec["barL1"].orDefault(0.f) == std::numeric_limits<float>::max());
    REQUIRE(sec["barL3"].orDefault(0.f) == INFINITY);
}

TEST_CASE("fail to parse field as float", "IniFile")
{
    std::istringstream ss("[Foo]"
			 "\nbar1=bla" "\nbar2=-2.5e4x" "\nbar3=");
    INIF
    //ini::IniFile inif(ss); 
      
    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED
    REQUIRE_THROWS_AS(sec["bar1"].as<float>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<float>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar3"].as<float>(), std::invalid_argument);
#endif
    
    REQUIRE(sec["bar1"].orDefault(.1f) == 0.1f);
    REQUIRE(sec["bar1"].failedLastOutConversion());
    REQUIRE(sec["bar2"].orDefault(.2f) == 0.2f);
    REQUIRE(sec["bar2"].failedLastOutConversion());
    REQUIRE(sec["bar3"].orDefault(.3f) == 0.3f);
    REQUIRE(sec["bar3"].failedLastOutConversion());
}

TEST_CASE("parse field as (unsigned) long int, fail if negative unsigned",
	  "IniFile")
{
    std::istringstream ss("[Foo]" "\nbar0=0" "\nbar1=1" "\nbar2=-42"
			  "\nbar80=00" "\nbar81=02" "\nbar82=-07"
			  "\nbarG0=-0x0" "\nbarG1=0xfF" "\nbarG2=-0x80"
			  "\nbarB1= 0x7fffffffffffffff" // max          long 
			  "\nbarB2= 0xffffffffffffffff" // max unsigned long 
			  "\nbarB3=-0x8000000000000000" // min          long 
			  "\nbarB4= 0x0"                // min unsigned long
			  "\nbarO1= 0x8000000000000000" // max          long + 1
			  "\nbarO2= 0x10000000000000000"// max unsigned long + 1
			  "\nbarO3=-0x8000000000000001");// min         long - 1
    INIF
    //ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED
    REQUIRE( sec["bar0"].as<         long int>() == 0);
    REQUIRE( sec["bar0"].as<unsigned long int>() == 0);
    REQUIRE( sec["bar1"].as<         long int>() == 1);
    REQUIRE( sec["bar1"].as<unsigned long int>() == 1);
    REQUIRE(          sec["bar2"].as<         long int>() == -42);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE( sec["bar80"].as<         long int>() == 0);
    REQUIRE( sec["bar80"].as<unsigned long int>() == 0);
    REQUIRE( sec["bar81"].as<         long int>() == 2);
    REQUIRE( sec["bar81"].as<unsigned long int>() == 2);
    REQUIRE(          sec["bar82"].as<long int>() == -7);
    REQUIRE_THROWS_AS(sec["bar82"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE(          sec["barG0"].as<long int>() == 0);
    REQUIRE_THROWS_AS(sec["barG0"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE(sec["barG1"].as<         long int>() == 255);
    REQUIRE(sec["barG1"].as<unsigned long int>() == 255);
    REQUIRE(          sec["barG2"].as<         long int>() == -128);
    // TBC: baG2 shall be NULL
    REQUIRE_THROWS_AS(sec["barG2"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE(sec["barB1"]       .as<         long int>()
	    == std::numeric_limits<         long int>::max());
    REQUIRE(sec["barB2"]       .as<unsigned long int>()
	    == std::numeric_limits<unsigned long int>::max());
    REQUIRE(sec["barB3"]       .as<         long int>()
	    == std::numeric_limits<         long int>::min());
    REQUIRE(sec["barB4"]       .as<unsigned long int>()
	    == std::numeric_limits<unsigned long int>::min());
    REQUIRE(sec["barO1"]       .as<         long int>() 
    	    == std::numeric_limits<         long int>::max());
    REQUIRE(sec["barO2"]       .as<unsigned long int>() 
    	    == std::numeric_limits<unsigned long int>::max());
    REQUIRE(sec["barO3"]       .as<         long int>() 
    	    == std::numeric_limits<         long int>::min());
#endif

    
    REQUIRE( sec["bar0"].orDefault(2l )          == 0);
    REQUIRE( sec["bar0"].orDefault(2lu)          == 0);
    REQUIRE(!sec["bar0"].failedLastOutConversion());
    REQUIRE( sec["bar1"].orDefault(-2l )         == 1);
    REQUIRE( sec["bar1"].orDefault(-2lu)         == 1);
    REQUIRE(!sec["bar1"].failedLastOutConversion());
    REQUIRE(sec["bar2"].orDefault(+2l )         == -42);
    REQUIRE(sec["bar2"].orDefault(+2lu)         ==   2);
    REQUIRE(sec["bar2"].failedLastOutConversion());
    REQUIRE( sec["bar80"].orDefault(+2l )         == 0);
    REQUIRE( sec["bar80"].orDefault(+2lu)         == 0);
    REQUIRE( sec["bar81"].orDefault(-2l )         == 2);
    REQUIRE( sec["bar81"].orDefault(-2lu)         == 2);
    REQUIRE(!sec["bar81"].failedLastOutConversion());
    REQUIRE(sec["bar82"].orDefault(+2l )          == -7);
    REQUIRE(sec["bar82"].orDefault(+2lu)          == +2);
    REQUIRE(sec["bar82"].failedLastOutConversion());
    REQUIRE(sec["barG0"].orDefault(+2l )         == 0);
    REQUIRE(sec["barG0"].orDefault(+2lu)         == 2);
    REQUIRE(sec["barG0"].failedLastOutConversion());
    REQUIRE(sec["barG1"].orDefault(-2l )         == 255);
    REQUIRE(sec["barG1"].orDefault(+2lu)         == 255);
    REQUIRE(sec["barG2"].orDefault(+2l )         == -128);
    REQUIRE(sec["barG2"].orDefault(+2lu)         ==    2);
    REQUIRE(sec["barG2"].failedLastOutConversion());
    REQUIRE(sec["barB1"]       .orDefault(-2l)
	    == std::numeric_limits<         long int>::max());
    REQUIRE(sec["barB2"]       .orDefault(+2lu)
	    == std::numeric_limits<unsigned long int>::max());
    REQUIRE(sec["barB3"]       .orDefault(+2l)
	    == std::numeric_limits<         long int>::min());
    REQUIRE(sec["barB4"]       .orDefault(+2lu)
	    == std::numeric_limits<unsigned long int>::min());
    REQUIRE(sec["barO1"]       .orDefault(+2l) 
    	    == std::numeric_limits<         long int>::max());
    REQUIRE(sec["barO2"]       .orDefault(+2lu) 
    	    == std::numeric_limits<unsigned long int>::max());
    REQUIRE(sec["barO3"]       .orDefault(+2l) 
    	    == std::numeric_limits<         long int>::min());
}


TEST_CASE("fail to parse field as (unsigned) long int", "IniFile")
{
    std::istringstream ss("[Foo]"
			  "\nbar1=bla" "\nbar2=" "\nbar3=2x" "\nbar4=+"
			  "\nbar82=08" "\nbarG0=0x" "\nbarG1=0xg"
			  );
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif(ss);
#endif

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED
    REQUIRE_THROWS_AS(sec["bar1"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar1"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar3"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar3"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar4"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar4"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar82"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar82"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG0"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG0"].as<unsigned long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG1"].as<         long int>(),
		      std::invalid_argument);
    REQUIRE_THROWS_AS(sec["barG1"].as<unsigned long int>(),
		      std::invalid_argument);
#endif

    // TBD: ensure that failure are really independent.
    // read in between some string. 
    REQUIRE(sec["bar1"].orDefault(+1l ) == 1l );
    REQUIRE(sec["bar1"].orDefault(+1lu) == 1lu);
    REQUIRE(sec["bar1"].failedLastOutConversion());

    REQUIRE(sec["bar2"].orDefault(+2l ) == 2l );
    REQUIRE(sec["bar2"].orDefault(+2lu) == 2lu);
    REQUIRE(sec["bar2"].failedLastOutConversion());
   
    REQUIRE(sec["bar3"].orDefault(+3l ) == 3l );
    REQUIRE(sec["bar3"].orDefault(+3lu) == 3lu);
    REQUIRE(sec["bar3"].failedLastOutConversion());
   
    REQUIRE(sec["bar4"].orDefault(+4l ) == 4l );
    REQUIRE(sec["bar4"].orDefault(+4lu) == 4lu);
    REQUIRE(sec["bar4"].failedLastOutConversion());

    REQUIRE(sec["bar82"].orDefault(+82l ) == 82l );
    REQUIRE(sec["bar82"].orDefault(+82lu) == 82lu);
    REQUIRE(sec["bar82"].failedLastOutConversion());
    
    REQUIRE(sec["barG0"].orDefault(+72l ) == 72l );
    REQUIRE(sec["barG0"].orDefault(+72lu) == 72lu);
    REQUIRE(sec["barG0"].failedLastOutConversion());

    REQUIRE(sec["barG1"].orDefault(+73l ) == 73l );
    REQUIRE(sec["barG1"].orDefault(+73lu) == 73lu);
    REQUIRE(sec["barG1"].failedLastOutConversion());
}


TEST_CASE("parse field as (unsigned) int, fail if negative unsigned",
	  "IniFile")
{
    std::istringstream ss("[Foo]" "\nbar0=0" "\nbar1=1" "\nbar2=-42"
			  "\nbar80=00" "\nbar81=02" "\nbar82=-07"
			  "\nbarG0=-0x0" "\nbarG1=0xfF" "\nbarG2=-0x80"
			  "\nbarB1= 0x7fffffff"  // max          int 
			  "\nbarB2= 0xffffffff"  // max unsigned int 
			  "\nbarB3=-0x80000000"  // min          int 
			  "\nbarB4= 0x0"         // min unsigned int
			  "\nbarO1= 0x80000000"  // max          int + 1
			  "\nbarO2= 0x100000000" // max unsigned int + 1
			  "\nbarO3=-0x80000001");// min          int - 1
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif(ss);
#endif

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED
    REQUIRE( sec["bar0"].as<         int>() == 0);
    REQUIRE( sec["bar0"].as<unsigned int>() == 0);
    REQUIRE( sec["bar1"].as<         int>() == 1);
    REQUIRE( sec["bar1"].as<unsigned int>() == 1);
    REQUIRE( sec["bar2"].as<         int>() == -42);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned int>(), std::invalid_argument);
    REQUIRE( sec["bar80"].as<         int>() == 0);
    REQUIRE( sec["bar80"].as<unsigned int>() == 0);
    REQUIRE( sec["bar81"].as<         int>() == 2);
    REQUIRE( sec["bar81"].as<unsigned int>() == 2);
    REQUIRE(          sec["bar82"].as<int>() == -7);
    REQUIRE_THROWS_AS(sec["bar82"].as<unsigned int>(), std::invalid_argument);
    REQUIRE(          sec["barG0"].as<int>() == 0);
    REQUIRE_THROWS_AS(sec["barG0"].as<unsigned int>(), std::invalid_argument);
    REQUIRE(sec["barG1"].as<         int>() == 255);
    REQUIRE(sec["barG1"].as<unsigned int>() == 255);
    REQUIRE(sec["barG2"].as<         int>() == -128);
    // TBC: baG2 shall be NULL
    REQUIRE_THROWS_AS(sec["baG2"].as<unsigned int>(), std::invalid_argument);
    REQUIRE(sec["barB1"]       .as<         int>()
	    == std::numeric_limits<         int>::max());
    REQUIRE(sec["barB2"]       .as<unsigned int>()
	    == std::numeric_limits<unsigned int>::max());
    REQUIRE(sec["barB3"]       .as<         int>()
	    == std::numeric_limits<         int>::min());
    REQUIRE(sec["barB4"]       .as<unsigned int>()
	    == std::numeric_limits<unsigned int>::min());
    REQUIRE(sec["barO1"]       .as<         int>() 
    	    == std::numeric_limits<         int>::max());
    REQUIRE(sec["barO2"]       .as<unsigned int>() 
    	    == std::numeric_limits<unsigned int>::max());
    REQUIRE(sec["barO3"]       .as<         int>() 
    	    == std::numeric_limits<         int>::min());
#endif


    REQUIRE( sec["bar0"].orDefault(2 )      == 0);
    REQUIRE( sec["bar0"].orDefault(2u)      == 0);
    REQUIRE(!sec["bar0"].failedLastOutConversion());
    REQUIRE( sec["bar1"].orDefault(-2 )     == 1);
    REQUIRE( sec["bar1"].orDefault(-2u)     == 1);
    REQUIRE(!sec["bar1"].failedLastOutConversion());
    REQUIRE(sec["bar2"].orDefault(+2 )     == -42);
    REQUIRE(sec["bar2"].orDefault(+2u)     ==   2);
    REQUIRE(sec["bar2"].failedLastOutConversion());
    REQUIRE( sec["bar80"].orDefault(+2 )     == 0);
    REQUIRE( sec["bar80"].orDefault(+2u)     == 0);
    REQUIRE( sec["bar81"].orDefault(-2 )     == 2);
    REQUIRE( sec["bar81"].orDefault(-2u)     == 2);
    REQUIRE(!sec["bar81"].failedLastOutConversion());
    REQUIRE(sec["bar82"].orDefault(+2 )      == -7);
    REQUIRE(sec["bar82"].orDefault(+2u)      == +2);
    REQUIRE(sec["bar82"].failedLastOutConversion());
    REQUIRE(sec["barG0"].orDefault(+2 )      == 0);
    REQUIRE(sec["barG0"].orDefault(+2u)      == 2);
    REQUIRE(sec["barG0"].failedLastOutConversion());
    REQUIRE(sec["barG1"].orDefault(-2 )     == 255);
    REQUIRE(sec["barG1"].orDefault(+2u)     == 255);
    REQUIRE(sec["barG2"].orDefault(+2 )     == -128);
    REQUIRE(sec["barG2"].orDefault(+2u)     ==    2);
    REQUIRE(sec["barG2"].failedLastOutConversion());
    REQUIRE(sec["barB1"]       .orDefault(-2)
	    == std::numeric_limits<         int>::max());
    REQUIRE(sec["barB2"]       .orDefault(+2u)
	    == std::numeric_limits<unsigned int>::max());
    REQUIRE(sec["barB3"]       .orDefault(+2)
	    == std::numeric_limits<         int>::min());
    REQUIRE(sec["barB4"]       .orDefault(+2u)
	    == std::numeric_limits<unsigned int>::min());
    REQUIRE(sec["barO1"]       .orDefault(+2) 
    	    == std::numeric_limits<         int>::max());
    REQUIRE(sec["barO2"]       .orDefault(+2u) 
    	    == std::numeric_limits<unsigned int>::max());
    REQUIRE(sec["barO3"]       .orDefault(+2) 
    	    == std::numeric_limits<         int>::min());
}


TEST_CASE("fail to parse field as (unsigned) int", "IniFile")
{
    std::istringstream ss("[Foo]"
			  "\nbar1=bla" "\nbar2=" "\nbar3=2x" "\nbar4=+"
			  "\nbar82=08" "\nbarG0=0x" "\nbarG1=0xg");
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif(ss);
#endif

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
#ifndef THROW_PREVENTED
    REQUIRE_THROWS_AS(sec["bar1"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar1"].as<unsigned int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<         int>(), std::invalid_argument);
    REQUIRE_THROWS_AS(sec["bar2"].as<unsigned int>(), std::invalid_argument);
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
#endif

    // TBD: ensure that failure are really independent.
    // read in between some string. 
    REQUIRE(sec["bar1"].orDefault(+1 ) == 1 );
    REQUIRE(sec["bar1"].orDefault(+1u) == 1u);
    REQUIRE(sec["bar1"].failedLastOutConversion());

    REQUIRE(sec["bar2"].orDefault(+2 ) == 2 );
    REQUIRE(sec["bar2"].orDefault(+2u) == 2u);
    REQUIRE(sec["bar2"].failedLastOutConversion());
   
    REQUIRE(sec["bar3"].orDefault(+3 ) == 3 );
    REQUIRE(sec["bar3"].orDefault(+3u) == 3u);
    REQUIRE(sec["bar3"].failedLastOutConversion());
   
    REQUIRE(sec["bar4"].orDefault(+4 ) == 4 );
    REQUIRE(sec["bar4"].orDefault(+4u) == 4u);
    REQUIRE(sec["bar4"].failedLastOutConversion());

    REQUIRE(sec["bar82"].orDefault(+82 ) == 82 );
    REQUIRE(sec["bar82"].orDefault(+82u) == 82u);
    REQUIRE(sec["bar82"].failedLastOutConversion());
    
    REQUIRE(sec["barG0"].orDefault(+72 ) == 72 );
    REQUIRE(sec["barG0"].orDefault(+72u) == 72u);
    REQUIRE(sec["barG0"].failedLastOutConversion());

    REQUIRE(sec["barG1"].orDefault(+73 ) == 73 );
    REQUIRE(sec["barG1"].orDefault(+73u) == 73u);
    REQUIRE(sec["barG1"].failedLastOutConversion());
}



TEST_CASE("parse field as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=true\nbar2=false\nbar3=tRuE");
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif(ss);
#endif

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
    REQUIRE( sec.size() == 3);

 #ifndef THROW_PREVENTED   
    REQUIRE( sec["bar1"].as<bool>());
    REQUIRE(!sec["bar2"].as<bool>());
    REQUIRE( sec["bar3"].as<bool>());
#endif

    REQUIRE( sec["bar1"].orDefault(false));
    REQUIRE(!sec["bar2"].orDefault(true ));
    REQUIRE( sec["bar3"].orDefault(false));
    
    REQUIRE(!sec["bar1"].failedLastOutConversion());
    REQUIRE(!sec["bar2"].failedLastOutConversion());
    REQUIRE(!sec["bar3"].failedLastOutConversion());
}

TEST_CASE("failed to parse field as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=yes");
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif(ss);
#endif

    REQUIRE(inif.size() == 1);
    ini::IniSection sec = inif["Foo"];
    //REQUIRE(inif["Foo"].size() == 3);
#ifndef THROW_PREVENTED
    REQUIRE_THROWS_AS(sec["bar"].as<bool>(), std::invalid_argument);
    REQUIRE( sec["bar"].as<std::string>() == std::string("yes"));
    REQUIRE(!sec["bar"].failedLastOutConversion());
    REQUIRE( sec["bar"].as<std::string>() == std::string("yes"));
    REQUIRE(!sec["bar"].failedLastOutConversion());
#endif
   
    REQUIRE(!sec["bar"].orDefault(false));
    REQUIRE( sec["bar"].failedLastOutConversion());
    REQUIRE( sec["bar"].orDefault(true ));
    REQUIRE( sec["bar"].failedLastOutConversion());
 }



TEST_CASE("parse field with custom field sep", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1:true\nbar2:false\nbar3:tRuE");
  
    ini::IniFile inif(':', '#');
#ifdef THROW_PREVENTED
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
   inif.decode(ss);
#endif

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE( inif["Foo"]["bar1"].asUnconditional<bool>());
    REQUIRE(!inif["Foo"]["bar2"].asUnconditional<bool>());
    REQUIRE( inif["Foo"]["bar3"].asUnconditional<bool>());
}

TEST_CASE("parse file with comment", "IniFile")
{
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    std::istringstream ss("[Foo]\n# this is a test\nbar=bla");
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
#else
    std::istringstream ss("[Foo]\n# this is a test\nbar=bla");
    ini::IniFile inif(ss);
#endif

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].asUnconditional<std::string>() == "bla");
}

TEST_CASE("parse with custom comment char", "IniFile")
{
    ini::IniFile inif('=', '$');
    std::istringstream ss("[Foo]\n$ this is a test\nbar=bla");
    bool isOk = inif.tryDecode(ss).isOk();
    REQUIRE(isOk);
    
    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].asUnconditional<std::string>() == "bla");
}

TEST_CASE("save with bool fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=true\nbar2=false\n");
}

TEST_CASE("save with (unsigned) (long) int fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] =  1u;
    inif["Foo"]["bar2"] = -2;
    inif["Foo"]["bar3"] =  3lu;
    inif["Foo"]["bar4"] = -4l;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=1\nbar2=-2\nbar3=3\nbar4=-4\n");
}

// TBD: infinite and NaN
TEST_CASE("save with double/float fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = 1.2;
    inif["Foo"]["bar2"] = -2.4;
    inif["Foo"]["bar3"] = -2.5f;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=1.2\nbar2=-2.4\nbar3=-2.5\n");
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
 * Treatment of spaces 
 ***************************************************/



TEST_CASE("spaces are not taken into account in field names", "IniFile")
{
    std::istringstream ss("[Foo]\n  \t  bar  \t  =hello world");
    INIF

    REQUIRE(inif["Foo"].find("bar") != inif["Foo"].end());
    REQUIRE(inif["Foo"]["bar"].toString() == "hello world");
}

TEST_CASE("spaces are not taken into account in field values", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=  \t  hello world  \t  ");
    INIF

    REQUIRE(inif["Foo"]["bar"].toString() == "hello world");
}

TEST_CASE("spaces are not taken into account in sections", "IniFile")
{
    std::istringstream ss("  \t  [Foo]  \t  \nbar=bla");
    INIF
    REQUIRE(inif.find("Foo") != inif.end());
}
