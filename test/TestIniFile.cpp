#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "IniFile.hpp"

#include <sstream>

TEST_CASE("succeed parse ini file", "IniFile")
{
    std::string toParse = "[Foo]\nbar=hello world\n[Test]";
    std::istringstream ss (toParse);
    ini::IniFile inif;
    inif.load(ss);

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"]["bar"].asString() == "hello world");
}
