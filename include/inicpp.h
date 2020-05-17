/*
 * inicpp.h
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#ifndef INICPP_H_
#define INICPP_H_

#include <algorithm>
#include <fstream>
#include <istream>
#include <map>
#include <sstream>
#include <stdexcept>

#include <iostream>
#include <limits>
#include <cmath>

namespace ini
{
    class IniField
    {
    private:
        std::string value_;
        mutable bool failedLastConversion_;
        mutable std::string typeLastConversion;

    public:
        IniField() : value_(), failedLastConversion_(false)
        {}

        IniField(const std::string &value) : value_(value)
        {}
        IniField(const IniField &field) : value_(field.value_)
        {}

        ~IniField()
        {}

        template<typename T>
        T as() const
        {
	    T result = static_cast<T>(*this);
	    if (failedLastConversion_)
	      throw std::invalid_argument("field is no " + typeLastConversion);

            return result;
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

        IniField &operator=(const IniField &field)
        {
            value_ = field.value_;
            return *this;
        }

        IniField &operator=(const int value)
        {
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
            return *this;
        }

        IniField &operator=(const unsigned int value)
        {
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
            return *this;
        }

        IniField &operator=(const double value)
        {
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
            return *this;
        }

        IniField &operator=(const float value)
        {
            std::stringstream ss;
            ss << value;
            value_ = ss.str();
            return *this;
        }

        IniField &operator=(const bool value)
        {
            if(value)
                value_ = "true";
            else
                value_ = "false";
            return *this;
        }

        /**********************************************************************
         * Cast Operators
         *********************************************************************/

        const char* castToCString() const 
        {
	    failedLastConversion_ = false;
            return value_.c_str();
	}
      
        explicit operator const char *() const
        {
	    typeLastConversion = "char*";
	    const char* result = castToCString();
 	    return result;
        }

        const std::string castToString() const 
        {
	    failedLastConversion_ = false;
            return value_;
	}
      
        explicit operator std::string() const
        {
	    typeLastConversion = "std::string";
	    const std::string result = castToString();
            return result;
        }

 	// strtol has a no thow guarantee 
        long int castToLongIntCheckFail() const 
        {
            char *endptr;
            long int result = std::strtol(value_.c_str(), &endptr, 0);
	    failedLastConversion_ = *endptr != '\0' || value_.empty();
	    return result;
 	}
      
        explicit operator long int() const
        {
	    typeLastConversion = "long int";
 	    long int result = castToLongIntCheckFail();
	    return result;
        }
      
        explicit operator int() const
        {
	    typeLastConversion = "int";
	    long int result = castToLongIntCheckFail();

	    if (result > std::numeric_limits<int>::max())
	      result = std::numeric_limits<int>::max();
	    else if (result < std::numeric_limits<int>::min())
	      result = std::numeric_limits<int>::min();
	    return (int)result;
        }

	// strtoul has a no thow guarantee 
        unsigned long int castToUnsignedLongIntCheckFail() const 
        {
 	    char *endptr;
	    // CAUTION: this delivers a value even if string starts with '-'
	    unsigned long int result = std::strtoul(value_.c_str(), &endptr, 0);
	    failedLastConversion_ =
	      *endptr != '\0' || value_.empty() || value_[0] == '-';
	    return result;
 	}

        explicit operator unsigned long int() const
        {
	    typeLastConversion = "unsigned long int";
	    unsigned long int result = castToUnsignedLongIntCheckFail();
	    return result;
        }

        explicit operator unsigned int() const
        {
	    typeLastConversion = "unsigned int";
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
	    failedLastConversion_ = *endptr != '\0' || value_.empty();
	    return result;
	}

        explicit operator double() const
        {
	    typeLastConversion = "double";
	    double result = castToDoubleCheckFail();
	    return result;
        }

	explicit operator float() const
        {
	    typeLastConversion = "float";
	    float result = (float)castToDoubleCheckFail();
	    return result;
	}



        explicit operator bool() const
        {
	    typeLastConversion = "bool";
            std::string str(value_);
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	    failedLastConversion_ = true;
	    bool result = false;
            if (str == "true")
	    {
	        failedLastConversion_ = false;
                result = true;
	    }
            if (str == "false")
	    {
 	        failedLastConversion_ = false;
	    }

	    return result;
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

    class IniFile : public std::map<std::string, IniSection>
    {
	//friend class DecodeResult;
    private:
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

	enum DecodeErrorCode
	{
	    NO_FAILURE = 0,
	    SECTION_NOT_CLOSED,
	    SECTION_EMPTY,
	    SECTION_TEXT_AFTER,
	    FIELD_WITHOUT_SECTION,
	    FIELD_WITHOUT_SEPARATOR
	};
	    
	class DecodeResult
	{
	    
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

	public:
	    // DecodeResult(DecodeErrorCode errorCode,
	    // 		 uint lineNumber,
	    // 		 std::string fileName)
	    // {
	    // 	this->errorCode = errorCode;
	    // 	this->lineNumber = lineNumber;
	    // 	this->fileName = fileName;
	    // }
	    
	    DecodeResult(DecodeErrorCode errorCode,
			 uint lineNumber) //:  DecodeResult(errorCode, lineNumber, (const char*)NULL)
	    {
	     	this->errorCode = errorCode;
	     	this->lineNumber = lineNumber;
	    }

	    DecodeResult() : DecodeResult(NO_FAILURE, -1)//, (const char*)NULL)
	    {
	    }

	    // TBD: close stream?
	    void throwIfError()
	    {
		std::stringstream ss;
		ss << "l" << this->lineNumber
		   << ": ini parsing failed, ";
		switch (this->errorCode)
		{
		case NO_FAILURE:
		    // all ok 
		    return;
		case SECTION_NOT_CLOSED:
		    ss << "section not closed";
		    break;
		case SECTION_EMPTY:
		    ss << "section is empty";
		    break;
		case SECTION_TEXT_AFTER:
		    ss << "no end of line after section";
		    break;
		case FIELD_WITHOUT_SECTION:
		    ss << "field has no section";
		    break;
		case FIELD_WITHOUT_SEPARATOR:
		    ss << "field without separator '" << //fieldSep_ << TBD: reactivated later
			"' found";
		    break;
		default:
		    ss << "unknown failure code " << this->errorCode << " found";
		    throw std::logic_error(ss.str());
		}
		// TBD: this shall be a kind of parse error 
		throw std::logic_error(ss.str());
	    }

	};
	
        IniFile() : IniFile('=', '#')
        {}

        IniFile(const char fieldSep, const char comment)
            : fieldSep_(fieldSep), comment_(comment)
        {}

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

        ~IniFile()
        {}

        void setFieldSep(const char sep)
        {
            fieldSep_ = sep;
        }

        void setCommentChar(const char comment)
        {
            comment_ = comment;
        }

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
	    DecodeResult* res = tryDecode(is);
	    res->throwIfError();
        }

        DecodeResult* tryDecode(std::istream &is)
	{
	    clear();
            int lineNo = 0;
            IniSection *currentSection = NULL;
            // iterate file by line
            while (!is.eof() && !is.fail())
            {
               std::string line;
                std::getline(is, line, '\n');
                trim(line);
                ++lineNo;

                // skip if line is empty
                if(line.size() == 0)
                    continue;

		// skip if line is a comment
                if(line[0] == comment_)
                    continue;
                if(line[0] == '[')
                {
                    // line is a section
                    // check if the section is also closed on same line
                    std::size_t pos = line.find("]");
                    if(pos == std::string::npos)
			return new DecodeResult(SECTION_NOT_CLOSED, lineNo);
                    // check if the section name is empty
                    if(pos == 1)
			return new DecodeResult(SECTION_EMPTY, lineNo);
                    // check if there is a newline following closing bracket
                    if(pos + 1 != line.length())
			return new DecodeResult(SECTION_TEXT_AFTER, lineNo);

                    // retrieve section name
                    std::string secName = line.substr(1, pos - 1);
                    currentSection = &((*this)[secName]);
                }
                else
                {
                    // line is a field definition
                    // check if section was already opened
                    if(currentSection == NULL)
			return new DecodeResult(FIELD_WITHOUT_SECTION, lineNo);

                    // find key value separator
                    std::size_t pos = line.find(fieldSep_);
                    if(pos == std::string::npos)
			return new DecodeResult(FIELD_WITHOUT_SEPARATOR, lineNo);

                    // retrieve field name and value
                    std::string name = line.substr(0, pos);
                    trim(name);
                    std::string value = line.substr(pos + 1, std::string::npos);
                    trim(value);
                    (*currentSection)[name] = value;
		}
	    }

	    // signifies success
	    return new DecodeResult();
	}	

	

        void decode(const std::string &content)
        {
            std::istringstream ss(content);
            decode(ss);
        }

        void load(const std::string &fileName)
        {
            std::ifstream is(fileName.c_str());
            decode(is);
        }

        void encode(std::ostream &os) const
        {
            // iterate through all sections in this file
            for(const auto &filePair : *this)
            {
                os << "[" << filePair.first << "]" << std::endl;
                // iterate through all fields in the section
                for(const auto &secPair : filePair.second)
                    os << secPair.first << fieldSep_
                       << secPair.second.as<std::string>() << std::endl;
            }
        }

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
    };
}

#endif
