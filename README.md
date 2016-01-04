# IniFileCpp

IniFileCpp is a simple and easy to use ini file en- and decoder for C++.

## Install

You can either build a static library using the CMake building system by running

```sh
cd <path-to-repo>
mkdir build
cd build
cmake ..
make
``` 

or simply copy the source files into your project and compile them directly.

## Usage

IniFileCpp allows loading data from any std::istream and requires a single function call or use the overloaded constructor.

```cpp
#include <IniFile.hpp>

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


For convenience there is also a ```load()``` function that expects a file name and parses the content of that file.

Sections and fields parsed from the stream can be accessed using the index operator [] and then be converted to various native types.

```cpp
bool myBool = myIni["Foo"]["myBool"].asBool();
std::string myStr = myIni["Foo"]["myStr"].asString();
int myInt = myIni["Foo"]["myInt"].asInt();
double myDouble = myIni["Foo"]["myDouble"].asDouble();
```

To create a ini file with IniCpp assign values to sections and fields. Supported types are:

* std::string
* int
* bool
* double

The content of the inifile can then be written to any std::ostream object.

```cpp
#include <IniFile.hpp>

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

For convenience there is also a ```save()``` function that expects a file name and writes the ini file to that file.
