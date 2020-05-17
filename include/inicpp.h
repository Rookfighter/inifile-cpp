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

    public:
        IniField() : value_()
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
            return static_cast<T>(*this);
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

        explicit operator const char *() const
        {
            return value_.c_str();
        }

        explicit operator std::string() const
        {
            return value_;
        }

	// strtol has a no thow guarantee 
        explicit operator long int() const
        {
            char *endptr;
            long int result = std::strtol(value_.c_str(), &endptr, 0);
	    if (*endptr != '\0' || value_.empty())
	        throw std::invalid_argument("field is no long int");
	    return result;
        }
      
	// strtol has a no thow guarantee 
        explicit operator int() const
        {
 	    long int result = this->as<long int>();
 	    
	    if (result > std::numeric_limits<int>::max())
	      result = std::numeric_limits<int>::max();
	    else if (result < std::numeric_limits<int>::min())
	      result = std::numeric_limits<int>::min();
	    return (int)result;
        }

	// strtoul has a no thow guarantee 
        explicit operator unsigned long int() const
        {
	    char *endptr;
	    // CAUTION: this delivers a value even if string starts with '-'
	    unsigned long int result = std::strtoul(value_.c_str(), &endptr, 0);
	    if (*endptr != '\0' || value_.empty() || value_[0] == '-')
	      throw std::invalid_argument("field is not an unsigned int");
	    return result;
        }

	// strtoul has a no thow guarantee 
        explicit operator unsigned int() const
        {
	    unsigned long int result = this->as<unsigned long int>();
	    
	    if (result > std::numeric_limits<unsigned int>::max())
	      result = std::numeric_limits<unsigned int>::max();
	    return result;
        }

 	// strtod would have a no throw guarantee 
	explicit operator float() const
        {
  	    return (float)this->as<double>();
        }

	// strtod has a no throw guarantee 
        explicit operator double() const
        {
	    char *endptr;
	    double result = std::strtod(value_.c_str(), &endptr);
	    if (*endptr != '\0' || value_.empty())
	      throw std::invalid_argument("field is no double");
	    return result;
        }

        explicit operator bool() const
        {
            std::string str(value_);
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);

            if (str == "TRUE")
                return true;
            if (str == "FALSE")
                return false;
	    throw std::invalid_argument("field is not a bool");
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
