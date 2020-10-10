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

        explicit operator int() const
        {
            char *endptr;
            // check if decimal
            int result = std::strtol(value_.c_str(), &endptr, 10);
            if(*endptr == '\0')
                return result;
            // check if octal
            result = std::strtol(value_.c_str(), &endptr, 8);
            if(*endptr == '\0')
                return result;
            // check if hex
            result = std::strtol(value_.c_str(), &endptr, 16);
            if(*endptr == '\0')
                return result;

            throw std::invalid_argument("field is not an int");
        }

        explicit operator unsigned int() const
        {
            char *endptr;
            // check if decimal
            int result = std::strtoul(value_.c_str(), &endptr, 10);
            if(*endptr == '\0')
                return result;
            // check if octal
            result = std::strtoul(value_.c_str(), &endptr, 8);
            if(*endptr == '\0')
                return result;
            // check if hex
            result = std::strtoul(value_.c_str(), &endptr, 16);
            if(*endptr == '\0')
                return result;

            throw std::invalid_argument("field is not an unsigned int");
        }

        explicit operator float() const
        {
            return std::stof(value_);
        }

        explicit operator double() const
        {
            return std::stod(value_);
        }

        explicit operator bool() const
        {
            std::string str(value_);
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);

            if(str == "TRUE")
                return true;
            else if(str == "FALSE")
                return false;
            else
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
    private:
        char fieldSep_;
        std::vector<std::string> commentPrefixes_;

        static void trim(std::string &str)
        {
            size_t startpos = str.find_first_not_of(" \t");
            if(std::string::npos != startpos)
            {
                size_t endpos = str.find_last_not_of(" \t");
                str = str.substr(startpos, endpos - startpos + 1);
            }
            else
                str = "";
        }

        void eraseComment(const std::string &commentPrefix,
            std::string &str,
            std::string::size_type startpos = 0)
        {
            size_t prefixpos = str.find(commentPrefix, startpos);
            if(std::string::npos == prefixpos)
                return;
            // Found a comment prefix, is it escaped?
            if(0 != prefixpos && '\\' == str[prefixpos - 1])
            {
                // The comment prefix is escaped, so just delete the escape char
                // and keep erasing after the comment prefix
                str.erase(prefixpos - 1, 1);
                eraseComment(
                    commentPrefix, str, prefixpos - 1 + commentPrefix.size());
            }
            else
            {
                str.erase(prefixpos);
            }
        }

        void eraseComments(std::string &str)
        {
            for(auto &commentPrefix : commentPrefixes_)
                eraseComment(commentPrefix, str);
        }

    public:
        IniFile() : IniFile('=', '#')
        {}

        IniFile(const char fieldSep, const char comment)
            : fieldSep_(fieldSep), commentPrefixes_(1, std::string(1, comment))
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

        IniFile(const char fieldSep,
            const std::vector<std::string> &commentPrefixes)
            : fieldSep_(fieldSep), commentPrefixes_(commentPrefixes)
        {}

        IniFile(const std::string &filename,
            const char fieldSep,
            const std::vector<std::string> &commentPrefixes)
            : fieldSep_(fieldSep), commentPrefixes_(commentPrefixes)
        {
            load(filename);
        }

        IniFile(std::istream &is,
            const char fieldSep,
            const std::vector<std::string> &commentPrefixes)
            : fieldSep_(fieldSep), commentPrefixes_(commentPrefixes)
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
            commentPrefixes_ = {std::string(1, comment)};
        }

        void setCommentPrefixes(const std::vector<std::string> &commentPrefixes)
        {
            commentPrefixes_ = commentPrefixes;
        }

        void decode(std::istream &is)
        {
            clear();
            int lineNo = 0;
            IniSection *currentSection = NULL;
            // iterate file by line
            while(!is.eof() && !is.fail())
            {
                std::string line;
                std::getline(is, line, '\n');
                eraseComments(line);
                trim(line);
                ++lineNo;

                // skip if line is empty
                if(line.size() == 0)
                    continue;
                if(line[0] == '[')
                {
                    // line is a section
                    // check if the section is also closed on same line
                    std::size_t pos = line.find("]");
                    if(pos == std::string::npos)
                    {
                        std::stringstream ss;
                        ss << "l" << lineNo
                           << ": ini parsing failed, section not closed";
                        throw std::logic_error(ss.str());
                    }
                    // check if the section name is empty
                    if(pos == 1)
                    {
                        std::stringstream ss;
                        ss << "l" << lineNo
                           << ": ini parsing failed, section is empty";
                        throw std::logic_error(ss.str());
                    }
                    // check if there is a newline following closing bracket
                    if(pos + 1 != line.length())
                    {
                        std::stringstream ss;
                        ss << "l" << lineNo
                           << ": ini parsing failed, no end of line after "
                              "section";
                        throw std::logic_error(ss.str());
                    }

                    // retrieve section name
                    std::string secName = line.substr(1, pos - 1);
                    currentSection = &((*this)[secName]);
                }
                else
                {
                    // line is a field definition
                    // check if section was already opened
                    if(currentSection == NULL)
                    {
                        std::stringstream ss;
                        ss << "l" << lineNo
                           << ": ini parsing failed, field has no section";
                        throw std::logic_error(ss.str());
                    }

                    // find key value separator
                    std::size_t pos = line.find(fieldSep_);
                    if(pos == std::string::npos)
                    {
                        std::stringstream ss;
                        ss << "l" << lineNo
                           << ": ini parsing failed, no '=' found";
                        throw std::logic_error(ss.str());
                    }
                    // retrieve field name and value
                    std::string name = line.substr(0, pos);
                    trim(name);
                    std::string value = line.substr(pos + 1, std::string::npos);
                    trim(value);
                    (*currentSection)[name] = value;
                }
            }
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
