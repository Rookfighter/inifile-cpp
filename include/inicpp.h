/*
 * inicpp.h
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#ifndef INICPP_H_
#define INICPP_H_

#include <fstream>
//#define SSTREAM_PREVENTED
//#ifndef SSTREAM_PREVENTED
#include <sstream>
//#endif

#include <limits.h>
#include <string.h>
// maybe alternative to iostream
#include <stdio.h>

#include <iostream>

#include <map>


// CAUTION: for embedded systems in general it is recommanded
// to set following switches:
// - THROW_PREVENTED disallows methods (and constructors) throwing exceptions
// - SSTREAM_PREVENTED

namespace ini
{
    /**
     * Represents a value in a key value pair of an ini-file. 
     */
    class IniField
    {
    private:
        /**
	 * Represents the value as a string which may be empty. 
	 */
        std::string value_;

        /**
	 * The type of the last outgoing cast conversion from value_. 
	 * This is insignificant and NULL if there was no out conversion yet. 
	 * This is used only to create an appropriate message 
	 * for exception in #as(). 
	 */
        mutable std::string typeLastOutConversion_;

        /**
	 * Whether the last outgoing cast conversion from value_ 
	 * e.g. into <c>int</> failed. 
	 * This is insignificant but false if there was no out conversion yet. 
	 * This is used by #as() to throw an exception 
	 * and by #orDefault(T) to set the defalt value. 
	 *
	 * @see failedLastOutConversion()
	 */
        mutable bool failedLastOutConversion_;

        /**
	 * Whether any ingoing cast conversion to value_ 
	 * e.g. from <c>int</> failed 
	 * since creation of this IniField 
	 * or reset of this flag via #resetFailedAnyInConversion(). 
	 * This is insignificant but false if there was no in conversion yet. 
	 * This is used by #as() to throw an exception 
	 * and by #orDefault(T) to set the defalt value. 
	 *
	 * @see failedAnyInConversion()
	 */
        bool failedAnyInConversion_;
      

    public:
      // TBC: needed? 
         IniField()
	   : value_(),
	     typeLastOutConversion_(),
	     failedLastOutConversion_(false),
	     failedAnyInConversion_(false)
          {}

        IniField(const std::string &value)
	  : value_(value),
	    typeLastOutConversion_(),
	    failedLastOutConversion_(false),
	    failedAnyInConversion_(false)
        {}
        IniField(const IniField &field)
	  : value_(field.value_),
	    typeLastOutConversion_  (field.typeLastOutConversion_),
	    failedLastOutConversion_(field.failedLastOutConversion_),
	    failedAnyInConversion_  (field.failedAnyInConversion_)

        {}

        ~IniField()
        {}

        // template<typename T>
        // T as() const
        // {
	//     T result = static_cast<T>(*this);
	//     if (failedLastOutConversion_)
	//       throw std::invalid_argument("field is no " + typeLastOutConversion_);

        //     return result;
        // }

        template<typename T>
        T asUnconditional() const
        {
	    return static_cast<T>(*this);
	}

#ifndef THROW_PREVENTED
        template<typename T>
        T as() const
        {
	    T result = asUnconditional<T>();
	    if (failedLastOutConversion_)
	      throw std::invalid_argument
		("field '" + value_ + "' is no " + typeLastOutConversion_);
            return result;
        }
#endif

        template<typename T>
        T orDefault(T defaultValue)
        {
	    T result = static_cast<T>(*this);
	    return failedLastOutConversion_ ? defaultValue : result;
 	}
      
        // template<typename T>
        // T &operator ||(T defaultValue)
        // {
	//     T result = static_cast<T>(*this);
	//     return failedLastOutConversion_ ? defaultValue : result;
 	// }

        const std::string toString() const
        {
	    return value_;
	}

        bool failedLastOutConversion()
        {
	  return failedLastOutConversion_;
	}

        /**
	 * Returns whether any inwards conversion by #convertNum10(char*, T) 
	 * failed since creation of this field or reset 
	 * done by #resetFailedAnyInConversion()
	 */
        bool failedAnyInConversion()
        {
	  return failedAnyInConversion_;
	}

      // TBC: this may be done in one with failedAnyInConversion()
        /**
	 * Resets #failedAnyInConversion_.
	 */
        void resetFailedAnyInConversion()
        {
	  failedAnyInConversion_ = false;
	}

        // TBD: works for decimal only; extend
      // NOTE: used for converting numbers only, both integers and floats.
      // Using sprintf is ok except for long int types:
      // then the trailing l must be removed. 
      // As in the cases under consideration, l occurs for long types only
      // and then it is at the end of the string,
      // it suffices to find pointer on 'l' and set '\0'. 
      // this works only because l occurs for long types T only.
      //
        /**
	 * Returns \p value as a string using the format string \p formatStr. 
	 * This must fit the type. 
	 * Note that for long int types, the result has no trailing 'l'. 
	 * As a side effect, conversion failures are cumulated 
	 * in #failedAnyInConversion_, which should not occur. 
	 *
	 * @param value
	 *    a numerical value, either integer or floating point. 
	 *    For integer both long int and int are avaliable 
	 *    and both signed and unsigned. 
	 *    For floating point both float and double are allowed. 
	 * @param formatStr
	 *    a format string which must fit the type. 
	 */
        template<typename T>
	std::string convertNum10(const char* formatStr, T value)
        {
	    char strVal[std::numeric_limits<T>::digits10];
	    int succ = sprintf(strVal, formatStr, value);
	    failedAnyInConversion_ |= succ < 0;
	    char* lChar = strchr(strVal, 'l');
	    if (lChar)
	        *lChar = '\0';
	    // TBD: evaluate succ: < 0 if sth went wrong.
	    // else number of variables written. should be 1
	    return std::string(strVal);
	}

      
        /**********************************************************************
         * Assignment Operators
         *********************************************************************/


        IniField &operator=(const char *value)
        {
            value_ = std::string(value);
            return *this;
        }

        IniField &operator=(const std::string &value)
        {
            value_ = value;
            return *this;
        }

        IniField &operator=(const int value)
        {
#ifdef SSTREAM_PREVENTED
	    value_ = convertNum10<int>("%d", value);
#else
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
#endif
            return *this;
        }

        IniField &operator=(const unsigned int value)
        {
#ifdef SSTREAM_PREVENTED
	    value_ = convertNum10<unsigned int>("%u", value);
#else
           std::stringstream ss;
            ss << value;
            value_ = ss.str();
#endif
           return *this;
        }

        IniField &operator=(const long int value)
        {
#ifdef SSTREAM_PREVENTED
	    value_ = convertNum10<long int>("%dl", value);
#else
           std::stringstream ss;
            ss << value;
            value_ = ss.str();
#endif
           return *this;
        }

      // TBD: clarify: this writes always decimal representation
      // although we can read different ones also (octal and hex)
      // that way conversions are not inverse to one another. 
        IniField &operator=(const unsigned long int value)
        {
#ifdef SSTREAM_PREVENTED
	    value_ = convertNum10<unsigned long int>("%ul", value);
#else
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
#endif
           return *this;
        }

        IniField &operator=(const double value)
        {
#ifdef SSTREAM_PREVENTED
	    value_ = convertNum10<double>("%g", value);
#else
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
#endif
            return *this;
        }

        IniField &operator=(const float value)
        {
#ifdef SSTREAM_PREVENTED
	    value_ = convertNum10<float>("%g", value);
#else
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
#endif
            return *this;
        }

        IniField &operator=(const bool value)
        {
	    value_ = value ? "true" : "false";
            return *this;
        }

        /**********************************************************************
         * Cast Operators
         *********************************************************************/

        const char* castToCString() const 
        {
	    failedLastOutConversion_ = false;
            return value_.c_str();
	}
      
        explicit operator const char *() const
        {
	    typeLastOutConversion_ = "char*";
	    const char* result = castToCString();
 	    return result;
        }

        const std::string castToString() const 
        {
	    failedLastOutConversion_ = false;
            return value_;
	}
      
        explicit operator std::string() const
        {
	    typeLastOutConversion_ = "std::string";
	    const std::string result = castToString();
            return result;
        }

 	// strtol has a no throw guarantee 
        long int castToLongIntCheckFail() const 
        {
            char *endptr;
            long int result = std::strtol(value_.c_str(), &endptr, 0);
	    failedLastOutConversion_ = *endptr != '\0' || value_.empty();
	    return result;
 	}

        /**
	 * Returns the value of this field, i.e. value_ 
	 * as a long int if possible; else returns 0. 
	 * As a side effect sets #typeLastOutConversion_. 
	 */
        explicit operator long int() const
        {
	    typeLastOutConversion_ = "long int";
 	    long int result = castToLongIntCheckFail();
	    return result;
        }
      
        explicit operator int() const
        {
	    typeLastOutConversion_ = "int";
	    long int result = castToLongIntCheckFail();

	    if (result > std::numeric_limits<int>::max())
	      result = std::numeric_limits<int>::max();
	    else if (result < std::numeric_limits<int>::min())
	      result = std::numeric_limits<int>::min();
	    return (int)result;
        }

	// strtoul has a no throw guarantee 
        unsigned long int castToUnsignedLongIntCheckFail() const 
        {
 	    char *endptr;
	    // CAUTION: this delivers a value even if string starts with '-'
	    unsigned long int result = std::strtoul(value_.c_str(), &endptr, 0);
	    failedLastOutConversion_ =
	      *endptr != '\0' || value_.empty() || value_[0] == '-';
	    return result;
 	}

        explicit operator unsigned long int() const
        {
	    typeLastOutConversion_ = "unsigned long int";
	    unsigned long int result = castToUnsignedLongIntCheckFail();
	    return result;
        }

        explicit operator unsigned int() const
        {
	    typeLastOutConversion_ = "unsigned int";
	    unsigned long int result = castToUnsignedLongIntCheckFail();

	    if (result > std::numeric_limits<unsigned int>::max())
	      result = std::numeric_limits<unsigned int>::max();
	    return result;
        }

 	// strtod has a no throw guarantee 
        double castToDoubleCheckFail() const 
        {
	    char *endptr;
	    double result = std::strtod(value_.c_str(), &endptr);
	    failedLastOutConversion_ = *endptr != '\0' || value_.empty();
	    return result;
	}

        explicit operator double() const
        {
	    typeLastOutConversion_ = "double";
	    double result = castToDoubleCheckFail();
	    return result;
        }

	explicit operator float() const
        {
	    typeLastOutConversion_ = "float";
	    float result = (float)castToDoubleCheckFail();
	    return result;
	}



        explicit operator bool() const
        {
	    typeLastOutConversion_ = "bool";
            std::string str(value_);
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            if (str == "true")
	    {
	        failedLastOutConversion_ = false;
                return true;
	    }
	    failedLastOutConversion_ = (str != "false");
	    return false;
        }
    };


    class IniSection : public std::map<std::string, IniField>
    {
    public:
        IniSection()
        {}
        ~IniSection()
        {}
    };

    /**
     * Enumeration of error codes which may occur during decoding an ini-file, 
     * whether as a stream, a string or a file. 
     */
    enum DecEncErrorCode
    {
        NO_FAILURE = 0,
	SECTION_NOT_CLOSED,
	SECTION_NAME_EMPTY,
	SECTION_TEXT_AFTER,
	ILLEGAL_LINE,
	FIELD_WITHOUT_SECTION,
	// TBD: occurs for file streams only, not for string streams
	STREAM_OPENR_FAILED,
	// TBD: occurs for file streams only, not for string streams
	STREAM_OPENW_FAILED,
	STREAM_READ_FAILED,
	STREAM_WRITE_FAILED
    };


    class IniFile : public std::map<std::string, IniSection>
    {
    public:
      	class DecEncResult
	{
	  friend IniFile;
	private:
	    /**
	     * This is DecEncErrorCode#NO_FAILURE if all ok so far. 
	     */
	    DecEncErrorCode errorCode;
	    /**
	     * This is -1 if no failure occurred yet. 
	     */
	    uint lineNumber;
	    /*
	     * This is <c>null</c> if reading from/writing to 
	     * a stream without file. 
	     */
	    //std::string fileName;
	    

	protected:
	    DecEncResult() 
	    {
	      reset();
	    }

	    void set(DecEncErrorCode errorCode, uint lineNumber)
	    {
	     	this->errorCode = errorCode;
	     	this->lineNumber = lineNumber;
	    }
	    void reset()
	    {
	      set(NO_FAILURE, -1);
	    }

	public:
	    DecEncErrorCode getErrorCode()
	    {
	        return errorCode;
	    }
	    uint getLineNumber()
	    {
	        return lineNumber;
	    }
	    bool isOk()
	    {
	      return this->errorCode == NO_FAILURE;
	    }

	}; // class DecEncResult

    private:
	const static char SEC_START = '[';
 	const static char SEC_END   = ']';

        DecEncResult deResult;
     
        char fieldSep_;
        char comment_;

        static void trim(std::string &str)
        {
            size_t startpos = str.find_first_not_of(" \t");
            if (std::string::npos != startpos)
            {
                size_t endpos = str.find_last_not_of(" \t");
                str = str.substr(startpos, endpos - startpos + 1);
            }
            else
                str = "";
        }

    public:

	
        IniFile() : IniFile('=', '#')
        {}

        IniFile(const char fieldSep, const char comment)
            : fieldSep_(fieldSep), comment_(comment)
        {}

        ~IniFile()
        {}


# ifndef THROW_PREVENTED
        IniFile(const std::string &filename,
            const char fieldSep = '=',
            const char comment = '#')
	  : IniFile(fieldSep, comment)
        {
	    load(filename);
        }

        IniFile(std::istream &is,
            const char fieldSep = '=',
            const char comment = '#')
	  : IniFile(fieldSep, comment)
        {
           decode(is);
         }

      // TBD: the above constructor shall be replaced by the ones below
      // to get the right exceptions. 
        IniFile(std::ifstream &is,
            const char fieldSep = '=',
            const char comment = '#')
	  : IniFile(fieldSep, comment)
        {
           decode(is);
	}
        IniFile(std::istringstream &is,
            const char fieldSep = '=',
            const char comment = '#')
	  : IniFile(fieldSep, comment)
        {
           decode(is);
	}
#endif


      

        void setFieldSep(const char sep)
        {
            fieldSep_ = sep;
        }

        void setCommentChar(const char comment)
        {
            comment_ = comment;
        }

        class InStreamInterface
	{
	public:
	  virtual bool isOpen() = 0;
	  virtual bool getLine(std::string &line) = 0;
	  virtual bool bad() = 0;
	  virtual int close() = 0;
	}; // class InStreamInterface

        // T is the kind of stream under consideration: ifstream or istringstream
        template<class T>
	class t_InStream : public InStreamInterface
	{
	protected:
	    T &iStream_;
	public:
	    t_InStream(T &iStream) : iStream_(iStream)
	    {
	    }
	    // overwritten for InFileStream
	    bool isOpen()
	    {
	      return true;
	    }
	    bool getLine(std::string &line)
	    {
	      return (bool)std::getline(iStream_, line, '\n');
	    }
	    // TBC: can this be false for an istringstream??
	    bool bad()
	    {
	        return iStream_.bad();
	    }
	   // overwritten for InFileStream
	    int close()
	    {
	        //iStream_.close();
	        return 0;
	    }
	};  // class t_InStream

        class InStringStream : public t_InStream<std::istringstream>
        {
	  // private:
	  // 	  static const std::istringstream str2str(const std::string str) 
	  // 	  {
	  // 	      std::istringstream ss(str);
	  // 	      return ss;
	  // 	  }
	public:
	
	    InStringStream(std::istringstream &iStream) : t_InStream(iStream)
	    {
	    }
	    // static InStringStream fromString(const std::string str)
  	    // {
	    //   std::istringstream ss(str);
	    //   InStringStream iss(ss);
	   //     return iss;
	    // }
	    // bool isOpen()
	    // {
	    //   return true;
	    // }
	    // int close()
	    // {
	    //     //iStream_.close();
	    // }
	};  // class InStringStream

      // TBD: clarify whether here also out of memory may occur
      // TBD: clarify memory leaks 
        class InStringStreamNS : public InStreamInterface
        {
	private:
	  char* str_;// TBD: eliminate
	public:
	  InStringStreamNS(std::string str)
	    {
	      int n = str.length();
	      str_ = (char*)malloc((n + 1)*sizeof(char)); 
	      strcpy(str_, str.c_str());
	      std::cout << "InStringStreamNS.init\n" << str_
			<< "'\n" << std::endl;

	    }
	    bool isOpen()
	    {
	        return true;
	    }
	    bool getLine(std::string &line)
	    {
		// std::cout << "InStringStreamNS.getLine str_ \n"
		// 	  << str_ << "'\n" << std::endl;
	        char* loc = strchr(str_, '\n');
	        if (loc == NULL)
	        {
		  //std::cout << "InStringStreamNS.getLine loc is NULL " << std::endl;
		    // no newline found 
		    if (*str_ == '\0')
		        return false;
		    std::string myline(str_);
		    line = myline;// TBD: eliminate hack 
		    //line(str_);
		    str_ = strchr(str_, '\0');
		    return true;
		}
		// std::cout << "InStringStreamNS.getLine loc \n"
		// 	  << loc 
		// 	  << "'" << std::endl;
		(*loc) = '\0';
		std::string myline(str_);
		str_=loc+1;
		line = myline;// TBD: eliminate hack 
	        // str_ += line;
	        // str_ += "\n";
		// std::cout << "InStringStreamNS.getLine out '"
		// 	  << line << "'" << std::endl;
	        return true;
	    }
	    bool bad()
	    {
	        return false;
	    }
	    int close()
	    {
	        return 0;
	    }
	}; // class InStringStreamNS


      
      
      class InFileStream : public t_InStream<std::ifstream>
      {
	public:
	    InFileStream(std::ifstream &iStream) : t_InStream(iStream)
	    {
	    }
	    bool isOpen()
	    {
	      return iStream_.is_open();
	    }
	    int close()
	    {
	        iStream_.close();
		return 0;
	    }
	};  // class InFileStream

        class InFileStreamNS : public InStreamInterface
        {
	private:
	      FILE* file;
	      char buff[255];// TBC: how to get rid of this uggly 255!!!
	      bool badBit;
	public:
	    InFileStreamNS(const std::string fName) //: str_(str)
	    {
	        file = fopen(fName.c_str(), "r");
		badBit = false;
	    }
	    bool isOpen()
	    {
	        return file != NULL;
	    }
	    bool getLine(std::string &line)
	    {
	        int numRead = fscanf(file, "%s\n", buff);
		// numRead can be 0 or 1
		if (numRead == 0)
		{
		    if (feof(file))
		        return false;
		    badBit = true;
		}
		line = "";// TBD: remove hack
		line += buff;
		std::cout << "InFileStreamNS.getLine" << numRead << std::endl;
	        return true;
	    }
	    bool bad()
	    {
	        return badBit;
	    }
	    int close()
	    {
	        return fclose(file);
	    }
	}; // class InFileStreamNS



        DecEncResult tryDecode(InStreamInterface &iStream)
	{
	    if (!iStream.isOpen())
	    {
	        deResult.set(STREAM_OPENR_FAILED, -1);
		return deResult;
	    }
	    clear();
            int lineNo = 1;
	    IniSection *currentSection = NULL;
	    for (std::string line; iStream.getLine(line); lineNo++)
            {
                trim(line);

                // skip if line is empty or a comment
                if(line.size() == 0 || line[0] == comment_)
                    continue;
		
                if(line[0] == SEC_START)
                {
                    // line is a section
                    // check if the section is also closed on same line
                    std::size_t pos = line.find(SEC_END);
                    if(pos == std::string::npos)
		    {
		        deResult.set(SECTION_NOT_CLOSED, lineNo);
		        return deResult;
		    }
                    // check if the section name is empty
                    if(pos == 1)
		    {
			deResult.set(SECTION_NAME_EMPTY, lineNo);
		        return deResult;
		    }
                     // check if there is a newline following closing bracket
                    if(pos + 1 != line.length())
		    {
			deResult.set(SECTION_TEXT_AFTER, lineNo);
		        return deResult;
		    }
                    // retrieve section name
                    std::string secName = line.substr(1, pos - 1);
                    currentSection = &((*this)[secName]);
                }
                else
                {
                    // find key value separator
                    std::size_t pos = line.find(fieldSep_);
                    if(pos == std::string::npos)
		    {
		        deResult.set(ILLEGAL_LINE, lineNo);
		        return deResult;
		    }
                    // line is a field definition
                    // check if section was already opened
                    if(currentSection == NULL)
		    {
		        deResult.set(FIELD_WITHOUT_SECTION, lineNo);
		        return deResult;
		    }

                    // retrieve field name and value
                    std::string name = line.substr(0, pos);
                    trim(name);
                    std::string value = line.substr(pos + 1, std::string::npos);
                    trim(value);
                    (*currentSection)[name] = value;
		}
	    }
	    // TBD: treat case where the stream fails.

	    // Note that the fail bit is set in conjunction with readline
	    // in this case always, but this does not indicate a failure,
	    // unlike the name may suggest and unlike bad bit. 
	    if (iStream.bad())
	    {
	      //std::cout << "bad  bit is set" << std::endl;
	      deResult.set(STREAM_READ_FAILED, lineNo);
	      return deResult;
	    }
	    // TBD: clarify
	    // TBD: take return value into account: maybe additional failures 
	    //iStream.close();

	    // signifies success
	    deResult.reset();
	    return deResult;
	}

        DecEncResult tryDecode(std::istream &iStream)
	{
	  t_InStream<std::istream> mystream(iStream);
	  return tryDecode(mystream);
	}
      
	DecEncResult tryDecode(const std::string &content)
	{
            std::istringstream ss(content);
	    InStringStream iss(ss);
            return tryDecode(iss);
 	}

        DecEncResult tryLoad(const std::string &fileName)
        {
            std::ifstream is(fileName.c_str());
	    InFileStream ifs(is);
            return tryDecode(ifs);
        }

        class OutStreamInterface
	{
	public:
	    virtual bool isOpen() = 0;
	    virtual OutStreamInterface& append(std::string str) = 0;
	    virtual OutStreamInterface& append(char ch) = 0;
	    virtual OutStreamInterface& appendNl() = 0;
	    virtual bool bad() = 0;
	    virtual void close() = 0;
	}; // class OutStreamInterface

        // T is the kind of stream under consideration: ofstream or ostringstream
        template<class T>
        class t_OutStream : public OutStreamInterface
	{
	protected:
	    T &oStream_;
	public:
	    t_OutStream(T &oStream) : oStream_(oStream)
	    {
	    }
	    // overwritten for OutFileStream
	    bool isOpen()
	    {
	      return true;
	    }
	    OutStreamInterface& append(std::string str)
	    {
	        oStream_ << str;
	        return *this;
	    }
	    OutStreamInterface& append(char ch)
	    {
	        oStream_ << ch;
	        return *this;
	    }
	    OutStreamInterface& appendNl()
	    {
	        oStream_ << std::endl;// defined in ostream
	        return *this;
	    }
	    // TBC: can this be false for an ostringstream??
	    bool bad()
	    {
	        return oStream_.bad();
	    }
	    // overwritten for OutFileStream
	    void close()
	    {
	        //iStream_.close();
	    }
        }; // class OutStream

        class OutStringStream : public t_OutStream<std::ostringstream>
      	{
      	public:
      	    OutStringStream(std::ostringstream &oStream) : t_OutStream(oStream)
      	    {
      	    }
      	    // overwritten for OutFileStream
      	    // bool isOpen()
      	    // {
      	    //   return true;
      	    // }
	    // void close()
	    // {
	    //     //oStream_.close();
	    // }
	}; // class OutStringStream

        class OutFileStream : public t_OutStream<std::ofstream>
     	{
     	public:
     	    OutFileStream(std::ofstream &oStream) : t_OutStream(oStream)
     	    {
     	    }
     	    // overwritten for OutFileStream
     	    bool isOpen()
     	    {
     	      return oStream_.is_open();
     	    }
     	    void close()
     	    {
     	        oStream_.close();
     	    }
        }; // class OutFileStream

 

     
        DecEncResult tryEncode(OutStreamInterface &oStream)
        {
	    if (!oStream.isOpen())
	    {
	        deResult.set(STREAM_OPENW_FAILED, -1);
		return deResult;
	    }
	    int lineNo = 1;
            // iterate through all sections in this file
            for (const auto &filePair : *this)
            {
	      
	        oStream.append(SEC_START).append(filePair.first).append(SEC_END)
		  .appendNl();
		lineNo++;
                // iterate through all fields in the section
                for (const auto &secPair : filePair.second)
		{
		    oStream.append(secPair.first            ).append(fieldSep_)
		      .     append(secPair.second.toString()).appendNl();
		    lineNo++;
		}
            }
	    
	    if (oStream.bad())
	    {
	      //std::cout << "bad  bit is set" << std::endl;
	      deResult.set(STREAM_WRITE_FAILED, lineNo);
	      return deResult;
	    }
	    // TBD: clarify 
	    //oStream.close();

	    // signifies success
	    deResult.reset();
	    return deResult;
        }
//#endif

//#ifndef SSTREAM_PREVENTED


        DecEncResult tryEncode(std::ostream &oStream)
	{
	    t_OutStream<std::ostream> mystream(oStream);
	    return tryEncode(mystream);
	}
      
	DecEncResult tryEncode(std::string &content)
	{
	    std::ostringstream ss;
	    OutStringStream oss(ss);
            DecEncResult res = tryEncode(ss);
	    content = ss.str();
	    return res;
 	}

        DecEncResult trySave(const std::string &fileName)
        {
            std::ofstream os(fileName.c_str());
	    OutFileStream ofs(os);
            return tryEncode(ofs);
        }


#ifndef THROW_PREVENTED
    private:

      	    // TBD: close stream?
	    void throwIfError(DecEncResult dRes)
	    {
		std::stringstream ss;
		ss << "l" << dRes.lineNumber
		   << ": ini parsing failed, ";
		switch (dRes.errorCode)
		{
		case NO_FAILURE:
		    // all ok 
		    return;
		case SECTION_NOT_CLOSED:
		    ss << "section not closed";
		    break;
		case SECTION_NAME_EMPTY:
		    ss << "section name is empty";
		    break;
		case SECTION_TEXT_AFTER:
		    ss << "no end of line after section";
		    break;
		case ILLEGAL_LINE:
		    ss << "found illegal line neither '"
		       << comment_ << "' comment, nor section nor field with separator '"
		       << fieldSep_ << "'";
		    break;
		case FIELD_WITHOUT_SECTION:
		    ss << "field has no section";
		    break;
		case STREAM_OPENR_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    ss << "could not open stream for read";
		    break;
		case STREAM_OPENW_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    ss << "could not open stream for write";
		    break;
		case STREAM_READ_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    ss << "because of stream read error found";
		    break;
		case STREAM_WRITE_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    ss << "because of stream write error found";
		    break;
		default:
		    ss << "unknown failure code " << dRes.errorCode << " found";
		    throw std::logic_error(ss.str());
		}
		// TBD: this shall be a kind of parse error 
		throw std::logic_error(ss.str());
	    }


    public:

      	/**
	 * @throws logic_error if 
	 *   - section not closed 
	 *   - section is empty 
	 *   - section: no end of line after section
	 *   - field has no section 
	 *   - field has no field separator 
	 */
        void decode(std::istream &is)
        {
	    throwIfError(tryDecode(is));
        }
      
        void decode(std::ifstream &is)
        {
	    InFileStream ifs(is);
	    throwIfError(tryDecode(ifs));
        }
      
        void decode(std::istringstream &is)
        {
	    InStringStream ifs(is);
	    throwIfError(tryDecode(ifs));
        }

        void decode(const std::string &content)
        {
	    throwIfError(tryDecode(content));
        }
      
        void load(const std::string &fileName)
        {
	    throwIfError(tryLoad(fileName));
        }

        /**
	 * @throws logic_error if 
	 * - 
	 */
        void encode(std::ostream &os)
        {
	  // OutStream os(oStream);
	  // // TBD: handle failures (return value)
	  // tryEncode(os);
	    throwIfError(tryEncode(os));
	}

        void encode(std::ofstream &os)
        {
	    OutFileStream ofs(os);
	    throwIfError(tryEncode(ofs));
        }
      
        void encode(std::ostringstream &os)
        {
	    OutStringStream ofs(os);
	    throwIfError(tryEncode(ofs));
        }


      

        std::string encode()
        {
            std::ostringstream ss;
            //encode(ss);
	    throwIfError(tryEncode(ss));
            return ss.str();
        }

        void save(const std::string &fileName)
        {
            // std::ofstream os(fileName.c_str());
            // encode(os);
	    throwIfError(trySave(fileName));
        }

#endif

    };
}

#endif
