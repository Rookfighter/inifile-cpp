#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "IniFile.hpp"

#include <sstream>

TEST_CASE("parse ini file", "IniFile")
{
    std::istringstream ss(("[Foo]\nbar=hello world\n[Test]"));
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"]["bar"].asString() == "hello world");
    REQUIRE(inif["Test"].size() == 0);
}

TEST_CASE("parse empty file", "IniFile")
{
    std::istringstream ss("");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 0);
}

TEST_CASE("parse section with duplicate field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello\nbar=world");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].asString() == "world");
}

TEST_CASE("parse field as double", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=1.2\nbar2=1\nbar3=-2.4");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].asDouble() == 1.2);
    REQUIRE(inif["Foo"]["bar2"].asDouble() == 1.0);
    REQUIRE(inif["Foo"]["bar3"].asDouble() == -2.4);
}

TEST_CASE("parse field as int", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=1\nbar2=-2");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 2);
    REQUIRE(inif["Foo"]["bar1"].asInt() == 1);
    REQUIRE(inif["Foo"]["bar2"].asInt() == -2);
}

TEST_CASE("parse field as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=true\nbar2=false\nbar3=tRuE");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].asBool());
    REQUIRE_FALSE(inif["Foo"]["bar2"].asBool());
    REQUIRE(inif["Foo"]["bar3"].asBool());
}
