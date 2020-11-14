# inifile-cpp
![License](https://img.shields.io/packagist/l/doctrine/orm.svg)
![Travis Status](https://travis-ci.org/Rookfighter/inifile-cpp.svg?branch=master)
![Appveyor Status](https://ci.appveyor.com/api/projects/status/xog92hd7x7t7ea2k?svg=true)

```inifile-cpp``` is a simple and easy to use single header-only ini file en- and decoder for C++.

## Install

Install the headers using the CMake build system:

```sh
cd <path-to-repo>
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make install
```

or simply copy the header file into your project and include it directly.

## Usage

For examples on how to use and extend (!) ```inifile-cpp``` for your custome needs, please have a look at the ```examples/``` directory.

```inifile-cpp``` allows loading data from any ```std::istream``` and requires a
single function call or use the overloaded constructor.

```cpp
#include <inicpp.h>

int main()
{
	// create istream object "is" ...

	// use function
	ini::IniFile myFirstIni;
	myFirstIni.decode(is);

	// or use the constructor
	ini::IniFile mySecondIni(is);
}
```

You can directly load ini-data from files by using the  ```load()``` function. It requires a file path
and automatically parses its contents:

```cpp
#include <inicpp.h>

int main()
{
    // load an ini file
	ini::IniFile myIni;
	myIni.load("some/ini/path");
}
```

Sections and fields can be accessed using the index operator ```[]```.
The values can be converted to various native types:

```cpp
bool myBool = myIni["Foo"]["myBool"].as<bool>();
char myChar = myIni["Foo"]["myChar"].as<char>();
unsigned char myUChar = myIni["Foo"]["myUChar"].as<unsigned char>();
int myInt = myIni["Foo"]["myInt"].as<int>();
unsigned int myUInt = myIni["Foo"]["myUInt"].as<unsigned int>();
long myLong = myIni["Foo"]["myLong"].as<long>();
unsigned long myULong = myIni["Foo"]["myULong"].as<unsigned long>();
float myFloat = myIni["Foo"]["myFloat"].as<float>();
double myDouble = myIni["Foo"]["myDouble"].as<double>();
std::string myStr = myIni["Foo"]["myStr"].as<std::string>();
const char *myStr2 = myIni["Foo"]["myStr"].as<const char*>();
```

Natively supported types are:

* ```bool```
* ```char```
* ```unsigned char```
* ```short```
* ```unsigned short```
* ```int```
* ```unsigned int```
* ```long```
* ```unsigned long```
* ```float```
* ```double```
* ```std::string```
* ```const char *```

Custom type conversions can be added by implementing specialized template of the ```ini::Convert<T>``` functor (see examples).

Values can be assigned to ini fileds just by using the assignment operator.
The content of the inifile can then be written to any ```std::ostream``` object.

```cpp
#include <inicpp.h>

int main()
{
	// create ostream object "os" ...

	ini::IniFile myIni;

	myIni["Foo"]["myInt"] = 1;
	myIni["Foo"]["myStr"] = "Hello world";
	myIni["Foo"]["myBool"] = true;
	myIni["Bar"]["myDouble"] = 1.2;

	myIni.encode(os);
}
```

You can directly svae ini-data to files by using the  ```save()``` function. It requires a file path
and automatically stores the ini file contents:

```cpp
#include <inicpp.h>

int main()
{
	ini::IniFile myIni;

    myIni["Foo"]["myInt"] = 1;
	myIni["Foo"]["myStr"] = "Hello world";
	myIni["Foo"]["myBool"] = true;
	myIni["Bar"]["myDouble"] = 1.2;

	myIni.save("some/ini/path");
}
```