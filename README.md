# Inifile-cpp
![](https://travis-ci.org/Rookfighter/inifile-cpp.svg?branch=master)
![](https://ci.appveyor.com/api/projects/status/xog92hd7x7t7ea2k?svg=true)

Inifile-cpp is a simple and easy to use header-only ini file en- and decoder for C++.

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

Inifile-cpp allows loading data from any ```std::istream``` and requires a
single function call or use the overloaded constructor.

```cpp
#include <inicpp.h>

int main()
{
	// create istream object "is" ...

	// use function
	ini::IniFile myIni;
	myIni.decode(is);

	// or use constructor
	// ini::IniFile myIni(is);
}
```


For convenience there is also a ```load()``` function that expects a file name
and parses the content of that file.

Sections and fields parsed from the stream can be accessed using the index
operator ```[]``` and then be converted to various native types.

```cpp
bool myBool = myIni["Foo"]["myBool"].as<bool>();
std::string myStr = myIni["Foo"]["myStr"].as<std::string>();
int myInt = myIni["Foo"]["myInt"].as<int>();
unsigned int myUInt = myIni["Foo"]["myUInt"].as<unsigned int>();
float myFloat = myIni["Foo"]["myFloat"].as<float>();
double myDouble = myIni["Foo"]["myDouble"].as<double>();
```

Natively supported types are:

* ```const char *```
* ```std::string```
* ```int```
* ```unsigned int```
* ```bool```
* ```float```
* ```double```

Custom type conversions can be added by implementing a explicit cast operator
for ```IniField```.

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

For convenience there is also a ```save()``` function that expects a file name
and writes the ini file to that file.
