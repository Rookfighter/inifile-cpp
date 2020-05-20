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
#define SSTREAM_PREVENTED
//#ifndef SSTREAM_PREVENTED
#include <sstream>
//#endif

#include <limits.h>
#include <string.h>

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
    enum DecodeErrorCode
    {
        NO_FAILURE = 0,
	SECTION_NOT_CLOSED,
	SECTION_NAME_EMPTY,
	SECTION_TEXT_AFTER,
	ILLEGAL_LINE,
	FIELD_WITHOUT_SECTION,
	// occurs for file streams only, not for string streams
	STREAM_OPEN_FAILED,
	STREAM_READ_FAILED
    };


    class IniFile : public std::map<std::string, IniSection>
    {
    public:
      	class DecodeResult
	{
	  friend IniFile;
	private:
	    /**
	     * This is DecodeErrorCode#NO_FAILURE if all ok so far. 
	     */
	    DecodeErrorCode errorCode;
	    /**
	     * This is -1 if no failure occurred yet. 
	     */
	    uint lineNumber;
	    /*
	     * This is <c>null</c> if reading from a stream without file. 
	     */
	    //std::string fileName;
	    

	protected:
	    DecodeResult() 
	    {
	      reset();
	    }

	    void set(DecodeErrorCode errorCode, uint lineNumber)
	    {
	     	this->errorCode = errorCode;
	     	this->lineNumber = lineNumber;
	    }
	    void reset()
	    {
	      set(NO_FAILURE, -1);
	    }

	public:
	    DecodeErrorCode getErrorCode()
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

	}; // class DecodeResult

    private:
	const static char SEC_START = '[';
 	const static char SEC_END   = ']';

        DecodeResult dResult;
     
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
	  virtual void close() = 0;
	}; // class InStreamInterface

        class InStream : public InStreamInterface
	{
	private:
	    std::istream &iStream_;
	public:
	    InStream(std::istream &iStream) : iStream_(iStream)
	    {
	    }
	    bool isOpen()
	    {
	      return true;
	    }
	    bool getLine(std::string &line)
	    {
	      return (bool)std::getline(iStream_, line, '\n');
	    }
	    bool bad()
	    {
	        return iStream_.bad();
	    }
	    void close()
	    {
	        //iStream_.close();
	    }
	};  // class InStream 

        DecodeResult tryDecode(InStreamInterface &iStream)
	{
	    if (!iStream.isOpen())
	    {
	        dResult.set(STREAM_OPEN_FAILED, -1);
		return dResult;
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
		        dResult.set(SECTION_NOT_CLOSED, lineNo);
		        return dResult;
		    }
                    // check if the section name is empty
                    if(pos == 1)
		    {
			dResult.set(SECTION_NAME_EMPTY, lineNo);
		        return dResult;
		    }
                     // check if there is a newline following closing bracket
                    if(pos + 1 != line.length())
		    {
			dResult.set(SECTION_TEXT_AFTER, lineNo);
		        return dResult;
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
		        dResult.set(ILLEGAL_LINE, lineNo);
		        return dResult;
		    }
                    // line is a field definition
                    // check if section was already opened
                    if(currentSection == NULL)
		    {
		        dResult.set(FIELD_WITHOUT_SECTION, lineNo);
		        return dResult;
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
	      std::cout << "bad  bit is set" << std::endl;
	      dResult.set(STREAM_READ_FAILED, lineNo);
	      return dResult;
	    }
	    // TBD: clarify 
	    //iStream.close();

	    // signifies success
	    dResult.reset();
	    return dResult;
	}

        DecodeResult tryDecode(std::istream &iStream)
	{
	  InStream mystream(iStream);
	  return tryDecode(mystream);
	}
      


	DecodeResult tryDecode(const std::string &content)
	{
            std::istringstream ss(content);
            return tryDecode(ss);
 	}


        DecodeResult tryLoad(const std::string &fileName)
        {
            std::ifstream is(fileName.c_str());
            return tryDecode(is);
        }

//#ifndef SSTREAM_PREVENTED
        void encode(std::ostream &os) const
        {
           // iterate through all sections in this file
            for(const auto &filePair : *this)
            {
                os << SEC_START << filePair.first << SEC_END << std::endl;
                // iterate through all fields in the section
                for(const auto &secPair : filePair.second)
                    os << secPair.first << fieldSep_
                       << secPair.second.toString() << std::endl;
            }
        }
//#endif

        std::string encode() const
        {
            std::ostringstream ss;
            encode(ss);
            return ss.str();
        }

        void save(const std::string &fileName) const
        {
            std::ofstream os(fileName.c_str());
            encode(os);
        }


#ifndef THROW_PREVENTED
    private:

      	    // TBD: close stream?
	    void throwIfError(DecodeResult dRes)
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
		case STREAM_READ_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    ss << "because of stream read error found";
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

        void decode(const std::string &content)
        {
	    throwIfError(tryDecode(content));
        }
      
        void load(const std::string &fileName)
        {
	    throwIfError(tryLoad(fileName));
        }
#endif

    };


}

#endif
