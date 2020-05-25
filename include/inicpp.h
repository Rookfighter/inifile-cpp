/*
 * inicpp.h
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#ifndef INICPP_H_
#define INICPP_H_

#ifndef SSTREAM_PREVENTED
#include <fstream>
#include <sstream>
#endif

// For debugging only
// but included from tests 
//#include <iostream>


#include <map>

#include <limits.h>
#include <string.h>

// maybe alternative to iostream
//#include <stdio.h>


// CAUTION: for embedded systems in general it is recommended
// to set following switches:
// - THROW_PREVENTED disallows methods (and constructors) throwing exceptions
// - SSTREAM_PREVENTED activates implementations not using streams
//   TBD: clarify consequences: above all lengths 

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
#ifdef SSTREAM_PREVENTED
        uint lengthText()
        {
	    uint res = 0;
	    for (const auto &pair : *this)
	    {
	        // for each entry length of key and of value
	        // plus 2 for separator and newline 
	        res += pair.first            .length()
		  + 2 +pair.second.toString().length();
	    }
	    return res;
        }
#endif
    };

    /**
     * Enumeration of error codes which may occur during decoding an ini-file, 
     * whether as a stream, a string or a file. 
     */
    enum DecEncErrorCode
    {
        // indicates that encoding/decoding finished without failure 
        NO_FAILURE = 0,
	// indicates that a section was not closed,
	// i.e. a line started with '[' but had no according ']'
	// in the course of decoding. 
	SECTION_NOT_CLOSED,
	// indicates that a section name was empty,
	// i.e. a line started with '[]'
	// in the course of decoding. 
	SECTION_NAME_EMPTY,
	// indicates that in a line defining a section there was text after ']'
	// i.e. ']' was not immediately followed by '\n' 
	// in the course of decoding. 
	SECTION_TEXT_AFTER,
	// indicates that a section name occurred more than once
	// in the course of decoding. 
	SECTION_NOT_UNIQUE,
	// indicates that during decoding an illegal line was found,
	// i.e. a line which is neither empty, 
	// (nor a comment lineby default starting with '#') 
	// nor a section line (starting with '[')
	// nor a key-value-line (containing by default '='). 
	ILLEGAL_LINE,
	// indicates that during decoding a field was found 
	// without preceeding section 
	FIELD_WITHOUT_SECTION,
	// indicates that during decoding a field was found 
	// with a key which is not unique within its section. 
	FIELD_NOT_UNIQUE_IN_SECTION,
	
	// TBD: occurs during decoding
	// of file streams only, not for string streams
	// if trying to open file which does not exist, or is not readable 
	STREAM_OPENR_FAILED,
	// TBD: occurs during encoding
	// for file streams only, not for string streams
	// if trying to open file which does not exist, or is not writable 
	STREAM_OPENW_FAILED,
	// during decoding if trying to read from a directory which is readable
	// maybe other cases 
	STREAM_READ_FAILED,
	// during encoding if trying to write to directory which is writable 
	// maybe other cases 
	STREAM_WRITE_FAILED
    };

    // class LightweightMap
    // {
    // };


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
	     * This is immaterial if no failure occurred yet. 
	     * It is set to 0 if the failure does not correspond 
	     * with a line number, 
	     * e.g. if open file fails. 
	     * Else it is the line number where the failure occurred. 
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
	     	this->errorCode  = errorCode;
		assert(this->lineNumber == lineNumber
		       && "line number out of sync");
	     	this->lineNumber = lineNumber;
	    }
	    void reset()
	    {
	      this->errorCode  = NO_FAILURE;
	      this->lineNumber = 0;
	      //set(NO_FAILURE, 0);
	    }
	    void incLineNo()
	    {
	        lineNumber++;
	    }
	public:
	    DecEncErrorCode getErrorCode()
	    {
	        return errorCode;
	    }
	    int getLineNumber()
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

 # ifndef SSTREAM_PREVENTED
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

#endif


      

        void setFieldSep(const char sep)
        {
            fieldSep_ = sep;
        }

        void setCommentChar(const char comment)
        {
            comment_ = comment;
        }

        /**
	 * 
	 */
        class InStreamInterface
	{
	public:
	  virtual bool isOpen() = 0;
	  virtual bool getLine(std::string &line) = 0;
	  virtual bool bad() = 0;
	  virtual int close() = 0;
	}; // class InStreamInterface

#ifdef SSTREAM_PREVENTED
      // TBD: clarify whether here also out of memory may occur
      // TBD: clarify memory leaks 
        /**
	 * 
	 */
        class InStringStreamNS : public InStreamInterface
        {
	private:
	  char* str_;
	public:
	  InStringStreamNS(std::string str)
	    {
	      int n = str.length();
	      str_ = (char*)malloc((n + 1)*sizeof(char)); 
	      strcpy(str_, str.c_str());
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
		    // no newline found 
		    if (*str_ == '\0')
		        return false;
		    std::string myline(str_);
		    line = myline;// TBD: eliminate hack 
		    //line(str_);
		    str_ = strchr(str_, '\0');
		    return true;
		}
		(*loc) = '\0';
		std::string myline(str_);
		str_=loc+1;
		line = myline;// TBD: eliminate hack 
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



     // TBD: unify InFileStreamNS and OutFileStreamNS
        /**
	 * 
	 */
        class InFileStreamNS : public InStreamInterface
        {
	private:
	  static const int LEN_LINE = 255;
	      FILE* file_;
	      char buff[LEN_LINE];// TBC: how to get rid of this uggly 255!!!
	      bool badBit;
	public:
	    InFileStreamNS(const std::string fName) //: str_(str)
	    {
	        file_ = fopen(fName.c_str(), "r");
		 // std::cout << "InFileStreamNS.open '"
		 // 	  << fName << "'" << std::endl;
		badBit = false;
	    }
	    bool isOpen()
	    {
	        return file_ != NULL;
	    }
	    bool getLine(std::string &line)
	    {
	        int numItemsRead;
		// skip empty lines 
		do
		{
	            numItemsRead = fscanf(file_, "%[\n]", buff);
		    if (ferror(file_))
		    {
		        badBit = true;
			return false;
		    }
		    if (numItemsRead == EOF)
		        return false;

		} while (numItemsRead == 1);

		// try to read lines closed by newline or eof (greedy)
	        numItemsRead = fscanf(file_, "%[^\n]", buff);
		if (ferror(file_))
		{
		    badBit = true;
		    return false;
		}
		if (numItemsRead == EOF)
		     return false;
		
		line = "";// TBD: remove hack
		line += buff;
		  // std::cout << "InFileStreamNS.getLine '"
		  // 	  << line << "'" << std::endl;
	        return true;
	    }
	    bool bad()
	    {
	        return badBit;
	    }
	    int close()
	    {
	        int res = fclose(file_);
		file_ = NULL;
	        return res;
	    }
	}; // class InFileStreamNS

#else
      // T is the kind of stream under consideration:
        // ifstream or istringstream
        /**
	 * 
	 */
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
	  // TBD: this shall be eliminated
	  // just to avoid tryDecode on general streams
	  // which may not be appropriate 
	    int close()
	    {
	      //iStream_.close();
	        return 0;
	    }
	};  // class t_InStream

        /**
	 * 
	 */
        class InStringStream : public t_InStream<std::istringstream>
        {
	public:
	
	    InStringStream(std::istringstream &iStream) : t_InStream(iStream)
	    {
	    }
	    // }
	    // bool isOpen()
	    // {
	    //   return true;
	    // }

	  // TBD: this shall be activated instead of template 
	    // int close()
	    // {
	    //   //iStream_.close();
	    //     return 0;
	    // }
	};  // class InStringStream

              /**
	 * 
	 */
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

#endif



        DecEncResult tryDecode(InStreamInterface &iStream)
	{
            int lineNo = 0;
	    deResult.reset();
	    if (!iStream.isOpen())
	    {
	        deResult.set(STREAM_OPENR_FAILED, lineNo);
		return deResult;
	    }
	    lineNo++;
	    deResult.incLineNo();
	    clear();
	    IniSection *currentSection = NULL;
	    for (std::string line;
		 iStream.getLine(line);
		 lineNo++,deResult.incLineNo())
            {
                trim(line);
		//std::cout << "decoding line " << line << std::endl;
		
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
                    // check if there is text
		    // between closing bracket and newline 
                    if(pos + 1 != line.length())
		    {
			deResult.set(SECTION_TEXT_AFTER, lineNo);
		        return deResult;
		    }
                    // retrieve section name
                    std::string secName = line.substr(1, pos - 1);
		    // std::cout << "section name '"
		    // 	      << secName << "'" << std::endl;
		    // check if section name occurred before 
		    if ((*this).count(secName) == 1)
		    {
		        deResult.set(SECTION_NOT_UNIQUE, lineNo);
		        return deResult;
		    }
                    currentSection = &((*this)[secName]);
		    //std::cout << " found section " << currentSection << std::endl;
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
		    //std::cout << " iskey-value " << std::endl;
                    // check if section was already opened
                    if(currentSection == NULL)
		    {
		        deResult.set(FIELD_WITHOUT_SECTION, lineNo);
		        return deResult;
		    }

                    // retrieve field key and value
                    std::string key = line.substr(0, pos);
                    trim(key);
                    std::string value = line.substr(pos + 1, std::string::npos);
                    trim(value);

		    // check if key name is  occurred before within the section
		    if ((*currentSection).count(key) == 1)
		    {
		        deResult.set(FIELD_NOT_UNIQUE_IN_SECTION, lineNo);
		        return deResult;
		    }
 
                    (*currentSection)[key] = value;
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
	    iStream.close();

	    // signifies success
	    deResult.reset();
	    return deResult;
	}

#ifndef SSTREAM_PREVENTED
      // TBC: with streams 
        DecEncResult tryDecode(std::istream &iStream)
	{
	  t_InStream<std::istream> mystream(iStream);
	  return tryDecode(mystream);
	}
#endif

      // TBD: alternatives: one with and one without streams 
	DecEncResult tryDecode(const std::string &content)
	{
#ifdef SSTREAM_PREVENTED
	    InStringStreamNS iss(content);
#else
	    std::istringstream ss(content);
	    InStringStream iss(ss);
#endif
            return tryDecode(iss);
 	}

      // TBD: one with and one without streams 
        DecEncResult tryLoad(const std::string &fileName)
        {
#ifdef SSTREAM_PREVENTED
	    InFileStreamNS ifs(fileName);
#else
            std::ifstream is(fileName.c_str());
	    InFileStream ifs(is);
#endif
            return tryDecode(ifs);
        }

        /**
	 * 
	 */
        class OutStreamInterface
	{
	public:
	    virtual bool isOpen() = 0;
	    virtual OutStreamInterface& append(std::string str) = 0;
	    virtual OutStreamInterface& append(char ch) = 0;
	    virtual OutStreamInterface& appendNl() = 0;
	    virtual std::string& str() = 0;
	    virtual bool bad() = 0;
	    virtual int close() = 0;
	}; // class OutStreamInterface

#ifdef SSTREAM_PREVENTED
      // TBD: check memory leaks 
        /**
	 * 
	 */
        class OutStringStreamNS : public OutStreamInterface
	{
	private:
	  char* str_;
	  char* ptr_;
	public:
	    OutStringStreamNS(uint len)
	    {
	      str_ = (char*)malloc(len*sizeof(char));
	      ptr_ = str_;
	    }
	    bool isOpen()
	    {
	        return true;
	    }
	    OutStreamInterface& append(std::string str)
	    {
	        strcpy(ptr_, str.c_str());
		ptr_ += str.length();
	      
		return *this;
	    }
	    OutStreamInterface& append(char ch)
	    {
	        (*ptr_) = ch;
		ptr_++;
		(*ptr_) = '\0';
		return *this;
	    }
	    OutStreamInterface& appendNl()
	    {
	        (*ptr_) = '\n';
		ptr_++;
		(*ptr_) = '\0';
		return *this;
	    }
	    std::string& str()
	    {
	      //str::string res(str_);
	      return *new std::string(str_);
	    }
	    bool bad()
	    {
	        return false;
	    }
	    int close()
	    {
	      (*ptr_) = '\0';
	        return 0;
	    }
	}; // class OutStringStreamNS


 
      // TBD: unify InFileStreamNS and OutFileStreamNS
        /**
	 * 
	 */
        class OutFileStreamNS : public OutStreamInterface
        {
	private:
	      FILE* file_;
	      char buff[255];// TBC: how to get rid of this uggly 255!!!
	      bool badBit;
	public:
	    OutFileStreamNS(const std::string fName) //: str_(str)
	    {
	        file_ = fopen(fName.c_str(), "w");
		badBit = false;
	    }
	    bool isOpen()
	    {
	        return file_ != NULL;
	    }
	    OutStreamInterface& append(std::string str)
	    {
	        int numCharsWritten = fprintf(file_, "%s", str.c_str());
		// TBD: more research 
		// numRead can be negative for failure, 0 or positive 
		if (numCharsWritten <= 0)
		{
		    // TBC: maybe this shall be str.length. 
		    if (!feof(file_))
		      badBit = true;
		}
	        return *this;
	    }
	    OutStreamInterface& append(char ch)
	    {
	        int numCharsWritten = fprintf(file_, "%c", ch);
		// TBD: more research 
		// numRead can be negative for failure, 0 or positive 
		if (numCharsWritten <= 0)
		{
		    // TBC: maybe this shall be str.length. 
		    if (!feof(file_))
		      badBit = true;
		}
	        return *this;
	    }
	  // TBD: interprete in terms oof append(char); not here 
	    OutStreamInterface& appendNl()
	    {
	        int numCharsWritten = fprintf(file_, "%c", '\n');
		// TBD: more research 
		// numRead can be negative for failure, 0 or positive 
		if (numCharsWritten <= 0)
		{
		    // TBC: maybe this shall be str.length. 
		    if (!feof(file_))
		      badBit = true;
		}
	        return *this;
	    }
	  // TBD: eliminate: bad design 
	    std::string& str()
	    {
	      //std::string res = "invalid; dont ask me";
	        return *new std::string("invalid; dont ask me");
	    }
	    bool bad()
	    {
	        return badBit;
	    }
	    int close()
	    {
	        int res = fclose(file_);
		file_ = NULL;
	        return res;
	    }
	}; // class OutFileStreamNS

      
#else
        // T is the kind of stream under consideration:
        // ofstream or ostringstream
        /**
	 * 
	 */
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
	  // TBD: eliminate: bad design 
	    std::string& str()
	    {
	        std::string res = "invalid; dont ask me";
	        //return oStream_.str();
	        return *new std::string("invalid; dont ask me");
	    }
	    // TBC: can this be false for an ostringstream??
	    bool bad()
	    {
	        return oStream_.bad();
	    }
	    // overwritten for OutFileStream
	    int close()
	    {
	        //iStream_.close();
	        return 0;
	    }
        }; // class t_OutStream


        /**
	 * 
	 */
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
	    std::string& str()
	    {
	      //std::string res = "invalid; dont ask me";
	         std::string res = oStream_.str();
		 return *new std::string(res);
	        //return *new std::string("invalid; dont ask me");
	    }
	    // void close()
	    // {
	    //     //oStream_.close();
	    // }
	}; // class OutStringStream

              /**
	 * 
	 */
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
	  // TBD: eliminate: bad design 
	    std::string& str()
	    {
	        return *new std::string("invalid; dont ask me");
	    }
     	    int close()
     	    {
     	        oStream_.close();
	        return 0;
     	    }
        }; // class OutFileStream

#endif


      
     
        DecEncResult tryEncode(OutStreamInterface &oStream)
        {
            int lineNo = 0;
	    deResult.reset();
	    if (!oStream.isOpen())
	    {
	        deResult.set(STREAM_OPENW_FAILED, lineNo);
		return deResult;
	    }
	    lineNo++;
	    deResult.incLineNo();
            // iterate through all sections in this file
            for (const auto &filePair : *this)
            {
	      
	        oStream.append(SEC_START)
		  .append(filePair.first).append(SEC_END)
		  .appendNl();
		deResult.incLineNo();
		lineNo++;
                // iterate through all fields in the section
                for (const auto &secPair : filePair.second)
		{
		    oStream.append(secPair.first            ).append(fieldSep_)
		      .     append(secPair.second.toString()).appendNl();
		    deResult.incLineNo();
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
	    // TBD: use return value 
	    oStream.close();

	    // signifies success
	    deResult.reset();
	    return deResult;
        }



#ifdef SSTREAM_PREVENTED
      // TBD: used only if SSTREAM_PREVENTED
        uint lengthText()
        {
	    uint res = 0;
	    for (auto &pair : *this)
	    {
	        // for each section length of name of section
	        // plus 2 for enclosing [...] plus 1 for newline 
	        // plus length of the section as such 
	        res += pair.first .length    ()
		  + 3 +pair.second.lengthText();
	    }
	    return res;
	}
 #else
    // TBC: with streams 
        DecEncResult tryEncode(std::ostream &oStream)
	{
	    t_OutStream<std::ostream> mystream(oStream);
	    return tryEncode(mystream);
	}
#endif
      
      // TBD: alternatives: one with and one without streams 
	DecEncResult tryEncode(std::string &content)
	{
#ifdef SSTREAM_PREVENTED
	    OutStringStreamNS oss(this->lengthText());
#else
	    std::ostringstream ss;
	    OutStringStream oss(ss);
#endif
            DecEncResult res = tryEncode(oss);
	    content = oss.str();
	    return res;
 	}

       // TBD: alternatives: one with and one without streams 
       DecEncResult trySave(const std::string &fileName)
        {
#ifdef SSTREAM_PREVENTED
	    OutFileStreamNS ofs(fileName);
#else
            std::ofstream os(fileName.c_str());
	    OutFileStream ofs(os);
#endif
            return tryEncode(ofs);
        }


#ifndef THROW_PREVENTED
    private:

      	    // TBD: close stream?
	    void throwIfError(DecEncResult dRes)
	    {
		std::string str = "";
	        if (dRes.lineNumber == 0)
		{
		    str += "without stream access";
		}
		else
		{
		    str += "in stream line ";
		    str += std::to_string(dRes.lineNumber);
		}
		str += ": ini parsing failed, ";
		switch (dRes.errorCode)
		{
		case NO_FAILURE:
		    // all ok 
		    return;
		case SECTION_NOT_CLOSED:
		    str += "section not closed";
		    break;
		case SECTION_NAME_EMPTY:
		    str += "section name is empty";
		    break;
		case SECTION_TEXT_AFTER:
		    str += "no end of line after section";
		    break;
		case ILLEGAL_LINE:
		  str += "found illegal line ";
		  str += "neither '";
		  str += comment_;
		  str += "'-comment, nor section nor field with separator '";
		  str += fieldSep_;
		  str += "'";
		    break;
		case FIELD_WITHOUT_SECTION:
		    str += "field has no section";
		    break;
		case STREAM_OPENR_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    str += "could not open stream for read";
		    break;
		case STREAM_OPENW_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    str += "could not open stream for write";
		    break;
		case STREAM_READ_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    str += "because stream read error occurred";
		    break;
		case STREAM_WRITE_FAILED:
		  // TBD: specified whether failbit or badbit is set. 
		    str += "because stream write error occurred";
		    break;
		default:
		  str += "unknown failure code ";
		  str += dRes.errorCode;
		  str += " found";
		    throw std::logic_error(str);
		}
		// TBD: this shall be a kind of parse error 
		throw std::logic_error(str);
	    }


    public:


      // TBC: shall be without streams 
        void decode(const std::string &content)
        {
	    throwIfError(tryDecode(content));
        }
      
      // TBC: tshall be without streams 
        void load(const std::string &fileName)
        {
	    throwIfError(tryLoad(fileName));
        }

              std::string encode()
        {
	  //std::ostringstream ss;
            //encode(ss);
	  std::string content;
	    throwIfError(tryEncode(content));
            //return ss.str();
	    return content;
        }

        void save(const std::string &fileName)
        {
            // std::ofstream os(fileName.c_str());
            // encode(os);
	    throwIfError(trySave(fileName));
        }


      
#ifndef SSTREAM_PREVENTED
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

        // void decode(std::ifstream &is)
        // {
	//     InFileStream ifs(is);
	//     throwIfError(tryDecode(ifs));
        // }
      
        // void decode(std::istringstream &is)
        // {
	//     InStringStream iss(is);
	//     throwIfError(tryDecode(iss));
        // }
      
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

        // void encode(std::ofstream &os)
        // {
	//     OutFileStream ofs(os);
	//     throwIfError(tryEncode(ofs));
        // }
      
        // void encode(std::ostringstream &os)
        // {
	//     OutStringStream ofs(os);
	//     throwIfError(tryEncode(ofs));
        // }

#endif

      


#endif
    };
}

#endif
