/*
 * IniFile.hpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#ifndef INIFILE_HPP_
#define INIFILE_HPP_

#include <map>
#include <istream>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <fstream>

namespace ini
{

    class IniField
    {
    private:
        std::string value_;
    public:
        IniField()
            : value_()
        {}

        IniField(const std::string &value)
            : value_(value)
        {}
        IniField(const IniField &field)
            : value_(field.value_)
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
            value_ = std::to_string(value);
            return *this;
        }

        IniField &operator=(const unsigned int value)
        {
            value_ = std::to_string(value);
            return *this;
        }

        IniField &operator=(const double value)
        {
            value_ = std::to_string(value);
            return *this;
        }

        IniField &operator=(const float value)
        {
            value_ = std::to_string(value);
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

         operator const char*() const
         {
             return value_.c_str();
         }

         operator std::string() const
         {
             return value_;
         }

         operator int() const
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

         operator unsigned int() const
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

         operator float() const
         {
             return std::stof(value_);
         }

         operator double() const
         {
             return std::stod(value_);
         }

         operator bool() const
         {
             std::string str(value_);
             std::transform(str.begin(), str.end(),str.begin(), ::toupper);

             if(str == "TRUE")
                 return true;
             else if(str == "FALSE")
                 return false;
             else
                 throw std::invalid_argument("field is not a bool");
         }
    };

    class IniSection: public std::map<std::string, IniField>
    {
    public:
        IniSection() { }
        ~IniSection() { }
    };

    class IniFile: public std::map<std::string, IniSection>
    {
    private:
        char fieldSep_;
        char comment_;
    public:
        IniFile(const char fieldSep = '=', const char comment = '#');
        IniFile(const std::string &fileName, const char fieldSep = '=',
                const char comment = '#');
        IniFile(std::istream &is, const char fieldSep = '=', const char comment = '#');
        ~IniFile();

        void setFieldSep(const char sep);
        void setCommentChar(const char comment);

        void decode(std::istream &is);
        void decode(const std::string &content);
        void encode(std::ostream &os);
        std::string encode();

        void load(const std::string &fileName);
        void save(const std::string &fileName);
    };
}

#endif
