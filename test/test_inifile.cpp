/*
 * test_inifile.cpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

//#include <catch.hpp>
// TBD: tests whether throws prevented or not. 
#include "inicpp.h"
#include <iostream>

#ifdef SSTREAM_PREVENTED
#define SS "SSY"
#else
#define SS "SSN"
#endif

#ifdef   THROW_PREVENTED
#define TH "THY"
#else
#define TH "THN"
#endif

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

#ifdef _WIN32
#include <io.h>
#else
// Here we assume that it is linux, else it does not work 
#include <unistd.h>
#endif

/* General tests of default strings: 
 * Failure code ini::DecEncErrorCode::NO_FAILURE
 */


TEST_CASE(TH " " SS " decode and encode ini string", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=hello world\n"
		    "# a comment\n"
		    "\n"
		    "[Test]\n\n");
    INIF

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"]["bar"].toString() == "hello world");
    REQUIRE(inif["Test"].size() == 0);

    
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryEncode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    str = inif.encode();
#endif

    // empty lines and comment lines are neglegted 
    REQUIRE(str ==
	    "[Foo]\n"
	    "bar=hello world\n"
	    "[Test]\n");
}

TEST_CASE(TH " " SS " decode and encode 2nd ini string", "IniFile")
{
    std::string str("[git]\n"
		    "revision=v0.0.1\n"
		    "dateTime=1968-03-11\n"
		    "[names]\n"
		    "nameId=GENERIC");
    INIF

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["git"]["revision"].toString() == "v0.0.1");
    REQUIRE(inif["git"]["dateTime"].toString() == "1968-03-11");
    REQUIRE(inif["names"]["nameId"].toString() == "GENERIC");
    REQUIRE(inif["git"].size() == 2);
    REQUIRE(inif["names"].size() == 1);

    #ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryEncode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    str = inif.encode();
#endif

    // gets reordered. 
    REQUIRE(str ==
	    "[git]\n"
	    "dateTime=1968-03-11\n"
	    "revision=v0.0.1\n"
	    "[names]\n"
	    "nameId=GENERIC\n");
}

TEST_CASE(TH " " SS " decode empty ini string", "IniFile")
{
    std::string str("");
    INIF
      // TBD: clarify whether it is ok to set an exception here:
      // stream has fail bit. 
      REQUIRE(inif.size() == 0);
}



/*
 * Test of failing running through all failure codes 
 */


TEST_CASE(TH " " SS " fail to decode ini string with section not closed", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=hello world\n"
		    "[Test\n"
		    "foo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    CHECK(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_NOT_CLOSED);
    CHECK(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}


TEST_CASE(TH " " SS " fail to decode ini string with empty section name", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=hello world\n"
		    "[]\n"
		    "foo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_NAME_EMPTY);
    REQUIRE(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}

TEST_CASE(TH " " SS " fail to decode ini string with text after section", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=hello world\n"
		    "[Test]superfluous\n"
		    "foo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_TEXT_AFTER);
    REQUIRE(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}

TEST_CASE(TH " " SS " fail to decode ini string with duplicate section",
	  "IniFile")
{
    std::string str("[Fox]\n"
		    "bar=hello world\n"
		    "[Test]\n"
		    "foo=irrelevant\n"
		    "[Fox]\n"
		    "foo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::SECTION_NOT_UNIQUE);
    REQUIRE(deResult.getLineNumber() == 5);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}



TEST_CASE(TH " " SS " fail to decode ini string with illegal line", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar no_separator\n"
		    "[Test]\n"
		    "foo=never reached");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::ILLEGAL_LINE);
    REQUIRE(deResult.getLineNumber() == 2);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}

TEST_CASE(TH " " SS " fail to decode ini string with field without custom separator", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=no_separator\n"
		    "[Test]\n"
		    "foo=never reached");
   ini::IniFile inif(':', '#');
#ifdef THROW_PREVENTED
     ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::ILLEGAL_LINE);
    REQUIRE(deResult.getLineNumber() == 2);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}



TEST_CASE(TH " " SS " fail to decode ini string with field without section", "IniFile")
{
    std::string str("# comment\n"
		    "bar=hello world\n"
		    "[Test]\n"
		    "foo=say goodby");
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::FIELD_WITHOUT_SECTION);
    REQUIRE(deResult.getLineNumber() == 2);
#else
    REQUIRE_THROWS_AS(inif.decode(str), std::logic_error);
#endif
}

TEST_CASE(TH " " SS " fail to decode ini string with duplicate field", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=hello\n"
		    "bar=World");


    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryDecode(str);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::FIELD_NOT_UNIQUE_IN_SECTION);
    REQUIRE(deResult.getLineNumber() == 3);
#else
    REQUIRE_THROWS_AS(inif.decode(str),  std::logic_error);
#endif
}


/*
 * Test failing with files 
 */




TEST_CASE(TH " " SS " fail load non-existing ini file", "IniFile")
{
    const std::string sfName = "doesNotExist.ini";
    const std::string fName = TESTFILE(sfName);
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    
    REQUIRE(res == -1);

    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryLoad(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::STREAM_OPENR_FAILED);
#else
    REQUIRE_THROWS_AS(inif.load(fName), std::logic_error);
#endif
}

TEST_CASE(TH " " SS " fail load/save directory as ini file", "IniFile")
{
    const std::string sfName = "dir.ini";
    const std::string fName = TESTFILE(sfName);
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == 0);
    std::filesystem::path path = fName;
    REQUIRE(std::filesystem::is_directory(path));

    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryLoad(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::STREAM_READ_FAILED);
#else
    REQUIRE_THROWS_AS(inif.load(fName), std::logic_error);
#endif

    
    inif.clear();
#ifdef THROW_PREVENTED
    deResult = inif.trySave(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::STREAM_OPENW_FAILED);
#else
    REQUIRE_THROWS_AS(inif.save(fName), std::logic_error);
#endif
}

TEST_CASE(TH " " SS " fail load unreadable as ini file", "IniFile")
{
    const std::string sfName = "writeOnly.ini";
    const std::string fName = TESTFILE(sfName);
    std::filesystem::path fPath = fName;
    
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == 0);
    
    std::filesystem::permissions(fPath, std::filesystem::perms::owner_write);
#ifdef _WIN32
    res = _access(fName.c_str(), 04);
#else
    res =  access(fName.c_str(), R_OK);
#endif
     REQUIRE(res == -1);
     
#ifdef _WIN32
    res = _access(fName.c_str(), 02);
#else
    res =  access(fName.c_str(), W_OK);
#endif
    REQUIRE(res == 0);
    
    std::filesystem::path path = fName;
    REQUIRE(std::filesystem::is_regular_file(path));

    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryLoad(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::STREAM_OPENR_FAILED);
#else
    REQUIRE_THROWS_AS(inif.load(fName), std::logic_error);
#endif

    std::filesystem::permissions(fPath,
				 std::filesystem::perms::owner_read|
				 std::filesystem::perms::owner_write,
				 std::filesystem::perm_options::add);
}

TEST_CASE(TH " " SS " fail save unwritable as ini file", "IniFile")
{
    const std::string sfName = "readOnly.ini";
    const std::string fName = TESTFILE(sfName);
    std::filesystem::path fPath = fName;
    
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == 0);
    std::filesystem::permissions(fPath, std::filesystem::perms::owner_read);
#ifdef _WIN32
    res = _access(fName.c_str(), 04);
#else
    res =  access(fName.c_str(), R_OK);
#endif
    REQUIRE(res == 0);
#ifdef _WIN32
    res = _access(fName.c_str(), 02);
#else
    res =  access(fName.c_str(), W_OK);
#endif
    REQUIRE(res == -1);
    std::filesystem::path path = fName;
    REQUIRE(std::filesystem::is_regular_file(path));

    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.trySave(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::STREAM_OPENW_FAILED);
#else
    REQUIRE_THROWS_AS(inif.save(fName), std::logic_error);
#endif
}

/*
 * Test of degenerate ini strings 
 */


TEST_CASE(TH " " SS " decode ini string with comment only", "IniFile")
{
    std::string str("# this is a comment");
    INIF

    REQUIRE(inif.size() == 0);
}


TEST_CASE(TH " " SS " decode ini string with empty section", "IniFile")
{
    std::string str("[Foo]");
    INIF

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 0);
}

/*
 * Test of successfully loading/saving files 
 */


TEST_CASE(TH " " SS " save and reload ini file", "IniFile")
{
    const std::string sfName = "example.ini";
    const std::string fName = TESTFILE(sfName);
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    switch (res)
    {
    case 0:
      // file exists and shall be deleted
      res = remove(fName.c_str());
      if (res != 0)
      {
	throw std::logic_error("Could not delete file.");
      }
      break;
    case -1:
      // file does not exist, all ok
      break;
    default:
      throw std::logic_error("Unexpected return value.");
    }
    // Here, sfName does not exist


#ifdef _WIN32
    res = _access(fName.c_str(), 00);
#else
    res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == -1);

    // save example inif as a file 
    ini::IniFile inif;
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;
    inif["Baz"]["bax0"] = 42;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.trySave(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    inif.save(fName);
#endif

    // check that the file exists
#ifdef _WIN32
    res = _access(fName.c_str(), 00);
#else
    res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == 0);

    // read back
    inif.clear();
#ifdef THROW_PREVENTED
    deResult = inif.tryLoad(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    inif.load(fName);
#endif

    // check result
    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"].size() == 2);
    REQUIRE(inif["Baz"].size() == 1);
#ifdef THROW_PREVENTED
    REQUIRE( inif["Foo"]["bar1"].asUnconditional<bool>() == true);
    REQUIRE( inif["Foo"]["bar2"].asUnconditional<bool>() == false);
    REQUIRE( inif["Baz"]["bax0"].asUnconditional<int>() == 42);
    REQUIRE(!inif["Foo"]["bar1"].failedLastOutConversion());
    REQUIRE(!inif["Foo"]["bar2"].failedLastOutConversion());
    REQUIRE(!inif["Baz"]["bax0"].failedLastOutConversion());
    
#else
    REQUIRE(inif["Foo"]["bar1"].as<bool>() == true);
    REQUIRE(inif["Foo"]["bar2"].as<bool>() == false);
    REQUIRE(inif["Baz"]["bax0"].as<int>() == 42);
#endif


    res = remove(fName.c_str());
    if (res != 0)
    {
	throw std::logic_error("Could not delete file.");
    }
}


TEST_CASE(TH " " SS " load ini file with trailing newline", "IniFile")
{
    const std::string sfName = "exampleNl.ini";
    const std::string fName = TESTFILE(sfName);
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == 0);
    
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryLoad(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    inif.load(fName);
#endif

    REQUIRE(inif.size() == 2);

#ifdef THROW_PREVENTED
    std::string result;
    deResult = inif.tryEncode(result);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    std::string result = inif.encode();
#endif
      REQUIRE(result ==
	      "[Bar]\n"
	      "bar0=0\n"
	      "[Foo]\n"
	      "foo1=44\n"
	      "foo2=true\n");
}

TEST_CASE(TH " " SS " load ini file with no trailing newline", "IniFile")
{
    const std::string sfName = "exampleNoNl.ini";
    const std::string fName = TESTFILE(sfName);
#ifdef _WIN32
    int res = _access(fName.c_str(), 00);
#else
    int res =  access(fName.c_str(), F_OK);
#endif
    REQUIRE(res == 0);
    
    ini::IniFile inif;
#ifdef THROW_PREVENTED
    ini::IniFile::DecEncResult deResult = inif.tryLoad(fName);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    inif.load(fName);
#endif

    REQUIRE(inif.size() == 2);

#ifdef THROW_PREVENTED
    std::string result;
    deResult = inif.tryEncode(result);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    std::string result = inif.encode();
#endif
      REQUIRE(result ==
	      "[Bar]\n"
	      "bar0=0\n"
	      "[Foo]\n"
	      "foo1=44\n"
	      "foo2=true\n");
}


/*
 * Test again degenerate ini strings 
 */


TEST_CASE(TH " " SS " parse empty field", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=");
    INIF

    CHECK(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].toString() == "");
}

// TEST_CASE(TH " " SS " parse non-existing field", "IniFile")
// {
//     std::string str("[Foo]\n"
// 		    "bar=");
//     INIF

//     CHECK(inif.size() == 1);
//     REQUIRE(inif["Foo"].size() == 1);
// should throw an exception 
//     REQUIRE(inif["Foo"]["bug"].toString() == "xx");
// }


/***************************************************
 * get as type
 ***************************************************/


TEST_CASE(TH " " SS " parse field as c-string", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=blaC");
    INIF
      
    const char* nvrOcc = "never occurs";
    REQUIRE(inif.size() == 1);
#ifndef THROW_PREVENTED
    REQUIRE(strcmp(inif["Foo"]["bar"].as<const char*>(), "blaC") == 0);
#endif
    REQUIRE(strcmp(inif["Foo"]["bar"].orDefault(nvrOcc), "blaC") == 0);
    REQUIRE(!inif["Foo"]["bar"].failedLastOutConversion());
}

TEST_CASE(TH " " SS " parse field as std::string", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=blaS");
    INIF

    const char* nvrOcc = "never occurs";

    REQUIRE(inif.size() == 1);
#ifndef THROW_PREVENTED
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == std::string("blaS"));
#endif
    REQUIRE(inif["Foo"]["bar"].orDefault(nvrOcc) == std::string("blaS"));
    REQUIRE(!inif["Foo"]["bar"].failedLastOutConversion());
}

TEST_CASE(TH " " SS " parse field as double", "IniFile")
{
    std::string str("[Foo]"
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

TEST_CASE(TH " " SS " fail to parse field as double", "IniFile")
{
   std::string str("[Foo]"
		   "\nbar1=bla" "\nbar2=-2.5e4x" "\nbar3=");
    INIF

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


TEST_CASE(TH " " SS " parse field as float", "IniFile")
{
    std::string str("[Foo]"
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

TEST_CASE(TH " " SS " fail to parse field as float", "IniFile")
{
    std::string str("[Foo]"
		    "\nbar1=bla" "\nbar2=-2.5e4x" "\nbar3=");
    INIF
      
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

TEST_CASE(TH " " SS " parse field as (unsigned) long int, fail if negative unsigned",
	  "IniFile")
{
    std::string str("[Foo]" "\nbar0=0" "\nbar1=1" "\nbar2=-42"
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


TEST_CASE(TH " " SS " fail to parse field as (unsigned) long int", "IniFile")
{
    std::string str("[Foo]"
		    "\nbar1=bla" "\nbar2=" "\nbar3=2x" "\nbar4=+"
		    "\nbar82=08" "\nbarG0=0x" "\nbarG1=0xg"
		    );
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(str).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif;
    inif.decode(str);
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


TEST_CASE(TH " " SS " parse field as (unsigned) int, fail if negative unsigned",
	  "IniFile")
{
    std::string str("[Foo]"
		    "\nbar0=0" "\nbar1=1" "\nbar2=-42"
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
    bool isOk = inif.tryDecode(str).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif;
    inif.decode(str);
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


TEST_CASE(TH " " SS " fail to parse field as (unsigned) int", "IniFile")
{
    std::string str("[Foo]"
		    "\nbar1=bla" "\nbar2=" "\nbar3=2x" "\nbar4=+"
		    "\nbar82=08" "\nbarG0=0x" "\nbarG1=0xg");
#ifdef THROW_PREVENTED
    ini::IniFile inif;
    bool isOk = inif.tryDecode(str).isOk();
    REQUIRE(isOk);
#else
    ini::IniFile inif;
    inif.decode(str);
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



TEST_CASE(TH " " SS " parse field as bool", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar1=true\n"
		    "bar2=false\n"
		    "bar3=tRuE");
INIF

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

TEST_CASE(TH " " SS " failed to parse field as bool", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=yes");
INIF
  
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



TEST_CASE(TH " " SS " parse field with custom field sep", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar1:true\n"
		    "bar2:false\n"
		    "bar3:tRuE");
  
    ini::IniFile inif(':', '#');
#ifdef THROW_PREVENTED
    bool isOk = inif.tryDecode(str).isOk();
    REQUIRE(isOk);
#else
   inif.decode(str);
#endif
    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE( inif["Foo"]["bar1"].asUnconditional<bool>());
    REQUIRE(!inif["Foo"]["bar2"].asUnconditional<bool>());
    REQUIRE( inif["Foo"]["bar3"].asUnconditional<bool>());
}

TEST_CASE(TH " " SS " parse file with comment", "IniFile")
{
    std::string str("[Foo]\n"
		    "# this is a test\n"
		    "bar=bla");
INIF

  REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].asUnconditional<std::string>() == "bla");
}

TEST_CASE(TH " " SS " parse with custom comment char", "IniFile")
{
    ini::IniFile inif('=', '$');
    std::string str("[Foo]\n"
		    "$ this is a test\n"
		    "bar=bla");
    bool isOk = inif.tryDecode(str).isOk();
    REQUIRE(isOk);
    
    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].asUnconditional<std::string>() == "bla");
}

TEST_CASE(TH " " SS " save with bool fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

#ifdef THROW_PREVENTED
    std::string result;
    ini::IniFile::DecEncResult deResult = inif.tryEncode(result);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    std::string result = inif.encode();
#endif
    REQUIRE(result ==
	    "[Foo]\n"
	    "bar1=true\n"
	    "bar2=false\n");
}

TEST_CASE(TH " " SS " save with (unsigned) (long) int fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] =  1u;
    inif["Foo"]["bar2"] = -2;
    inif["Foo"]["bar3"] =  3lu;
    inif["Foo"]["bar4"] = -4l;

#ifdef THROW_PREVENTED
    std::string result;
    ini::IniFile::DecEncResult deResult = inif.tryEncode(result);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    std::string result = inif.encode();
#endif
    REQUIRE(result == "[Foo]\nbar1=1\nbar2=-2\nbar3=3\nbar4=-4\n");
}

// TBD: infinite and NaN
TEST_CASE(TH " " SS " save with double/float fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = 1.2;
    inif["Foo"]["bar2"] = -2.4;
    inif["Foo"]["bar3"] = -2.5f;

#ifdef THROW_PREVENTED
    std::string result;
    ini::IniFile::DecEncResult deResult = inif.tryEncode(result);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    std::string result = inif.encode();
#endif
    REQUIRE(result == "[Foo]\nbar1=1.2\nbar2=-2.4\nbar3=-2.5\n");
}

TEST_CASE(TH " " SS " save with custom field sep", "IniFile")
{
    ini::IniFile inif(':', '#');
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

#ifdef THROW_PREVENTED
    std::string result;
    ini::IniFile::DecEncResult deResult = inif.tryEncode(result);
    REQUIRE(deResult.getErrorCode()
	    == ini::DecEncErrorCode::NO_FAILURE);
#else
    std::string result = inif.encode();
#endif
    REQUIRE(result == "[Foo]\nbar1:true\nbar2:false\n");
}





/***************************************************
 * Treatment of spaces 
 ***************************************************/



TEST_CASE(TH " " SS " spaces are not taken into account in field names", "IniFile")
{
    std::string str("[Foo]\n"
		    "  \t  bar  \t  =hello world");
    INIF

    REQUIRE(inif["Foo"].find("bar") != inif["Foo"].end());
    REQUIRE(inif["Foo"]["bar"].toString() == "hello world");
}

TEST_CASE(TH " " SS " spaces are not taken into account in field values", "IniFile")
{
    std::string str("[Foo]\n"
		    "bar=  \t  hello world  \t  ");
    INIF

    REQUIRE(inif["Foo"]["bar"].toString() == "hello world");
}

TEST_CASE(TH " " SS " spaces are not taken into account in sections", "IniFile")
{
    std::string str("  \t  [Foo]  \t  \n"
		    "bar=bla");
    INIF

// #ifdef THROW_PREVENTED
//   ini::IniFile inif;
//   bool isOk = inif.tryDecode(str).isOk();
//   REQUIRE(isOk);				
// #else
//     ini::IniFile inif(str);
// #endif

    REQUIRE(inif.find("Foo") != inif.end());
}


#undef SS
#undef TH

#ifdef SSTREAM_PREVENTED
#undef SSTREAM_PREVENTED
#endif

#ifdef   THROW_PREVENTED
#undef   THROW_PREVENTED
#endif

#undef INIF
