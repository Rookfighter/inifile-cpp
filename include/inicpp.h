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
#include <assert.h>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ini
{
    /************************************************
     * Conversion Functors
     ************************************************/

    inline bool strToLong(const std::string &value, long &result)
    {
        char *endptr;
        // check if decimal
        result = std::strtol(value.c_str(), &endptr, 10);
        if(*endptr == '\0')
            return true;
        // check if octal
        result = std::strtol(value.c_str(), &endptr, 8);
        if(*endptr == '\0')
            return true;
        // check if hex
        result = std::strtol(value.c_str(), &endptr, 16);
        if(*endptr == '\0')
            return true;

        return false;
    }

    inline bool strToULong(const std::string &value, unsigned long &result)
    {
        char *endptr;
        // check if decimal
        result = std::strtoul(value.c_str(), &endptr, 10);
        if(*endptr == '\0')
            return true;
        // check if octal
        result = std::strtoul(value.c_str(), &endptr, 8);
        if(*endptr == '\0')
            return true;
        // check if hex
        result = std::strtoul(value.c_str(), &endptr, 16);
        if(*endptr == '\0')
            return true;

        return false;
    }

    template<typename T>
    struct Convert
    {};

    template<>
    struct Convert<bool>
    {
        void decode(const std::string &value, bool &result)
        {
            std::string str(value);
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);

            if(str == "TRUE")
                result = true;
            else if(str == "FALSE")
                result = false;
            else
                throw std::invalid_argument("field is not a bool");
        }

        void encode(const bool value, std::string &result)
        {
            result = value ? "true" : "false";
        }
    };

    template<>
    struct Convert<char>
    {
        void decode(const std::string &value, char &result)
        {
            assert(value.size() > 0);
            result = value[0];
        }

        void encode(const char value, std::string &result)
        {
            result = value;
        }
    };

    template<>
    struct Convert<unsigned char>
    {
        void decode(const std::string &value, unsigned char &result)
        {
            assert(value.size() > 0);
            result = value[0];
        }

        void encode(const unsigned char value, std::string &result)
        {
            result = value;
        }
    };

    template<>
    struct Convert<short>
    {
        void decode(const std::string &value, short &result)
        {
            long tmp;
            if(!strToLong(value, tmp))
                throw std::invalid_argument("field is not a short");
            result = static_cast<short>(tmp);
        }

        void encode(const short value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<unsigned short>
    {
        void decode(const std::string &value, unsigned short &result)
        {
            unsigned long tmp;
            if(!strToULong(value, tmp))
                throw std::invalid_argument("field is not an unsigned short");
            result = static_cast<unsigned short>(tmp);
        }

        void encode(const unsigned short value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<int>
    {
        void decode(const std::string &value, int &result)
        {
            long tmp;
            if(!strToLong(value, tmp))
                throw std::invalid_argument("field is not an int");
            result = static_cast<int>(tmp);
        }

        void encode(const int value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<unsigned int>
    {
        void decode(const std::string &value, unsigned int &result)
        {
            unsigned long tmp;
            if(!strToULong(value, tmp))
                throw std::invalid_argument("field is not an unsigned int");
            result = static_cast<unsigned int>(tmp);
        }

        void encode(const unsigned int value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<long>
    {
        void decode(const std::string &value, long &result)
        {
            if(!strToLong(value, result))
                throw std::invalid_argument("field is not a long");
        }

        void encode(const long value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<unsigned long>
    {
        void decode(const std::string &value, unsigned long &result)
        {
            if(!strToULong(value, result))
                throw std::invalid_argument("field is not an unsigned long");
        }

        void encode(const unsigned long value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<double>
    {
        void decode(const std::string &value, double &result)
        {
            result = std::stod(value);
        }

        void encode(const double value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<float>
    {
        void decode(const std::string &value, float &result)
        {
            result = std::stof(value);
        }

        void encode(const float value, std::string &result)
        {
            std::stringstream ss;
            ss << value;
            result = ss.str();
        }
    };

    template<>
    struct Convert<std::string>
    {
        void decode(const std::string &value, std::string &result)
        {
            result = value;
        }

        void encode(const std::string &value, std::string &result)
        {
            result = value;
        }
    };

    template<>
    struct Convert<const char*>
    {
        void encode(const char* const &value, std::string &result)
        {
            result = value;
        }

        void decode(const std::string &value, const char* &result)
        {
            result = value.c_str();
        }
    };

    template<>
    struct Convert<char*>
    {
        void encode(const char* const &value, std::string &result)
        {
            result = value;
        }
    };

    template<size_t n>
    struct Convert<char[n]>
    {
        void encode(const char *value, std::string &result)
        {
            result = value;
        }
    };

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
            Convert<T> conv;
            T result;
            conv.decode(value_, result);
            return result;
        }

        template<typename T>
        IniField &operator=(const T &value)
        {
            Convert<T> conv;
            conv.encode(value, value_);
            return *this;
        }

        IniField &operator=(const IniField &field)
        {
            value_ = field.value_;
            return *this;
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
