/*
 * IniFile.cpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#include "IniFile.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <iostream>

#define TO_UPPER(str) std::transform(str.begin(), str.end(),str.begin(), ::toupper)

namespace ini
{

    /************************************
     *          Helper Functions
     ************************************/

    static bool strToBool(const std::string &value)
    {
        std::string tmp(value);
        TO_UPPER(tmp);

        if(tmp == "TRUE")
            return true;
        else if(tmp == "FALSE")
            return false;
        else
            throw std::domain_error("field is not a bool");
    }

    static int strToInt(const std::string &value)
    {
        char *endptr;
        int result = strtol(value.c_str(), &endptr, 10);
        if(*endptr != '\0')
            throw std::domain_error("field is not an int");

        return result;
    }

    static double strToDouble(const std::string &value)
    {
        char *endptr;
        double result = strtod(value.c_str(), &endptr);
        if(*endptr != '\0')
            throw std::domain_error("field is not a double");

        return result;
    }

    /************************************
     *          IniField
     ************************************/

    IniField::IniField()
    {
    }

    IniField::IniField(const std::string &value)
        : value_(value)
    {

    }

    IniField::IniField(const IniField &field)
        : value_(field.value_)
    {
    }

    IniField::~IniField()
    {
    }

    const std::string &IniField::asString() const
    {
        return value_;
    }

    int IniField::asInt() const
    {
        return strToInt(value_);
    }

    unsigned int IniField::asUInt() const
    {
        return static_cast<unsigned int>(strToInt(value_));
    }

    double IniField::asDouble() const
    {
        return strToDouble(value_);
    }

    float IniField::asFloat() const
    {
        return static_cast<float>(strToDouble(value_));
    }

    bool IniField::asBool() const
    {
        return strToBool(value_);
    }

    IniField &IniField::operator=(const IniField &field)
    {
        value_ = field.value_;
        return *this;
    }

    IniField &IniField::operator=(const int value)
    {
        std::stringstream ss;
        ss << value;
        value_ = ss.str();
        return *this;
    }

    IniField &IniField::operator=(const unsigned int value)
    {
        std::stringstream ss;
        ss << value;
        value_ = ss.str();
        return *this;
    }

    IniField &IniField::operator=(const double value)
    {
        std::stringstream ss;
        ss << value;
        value_ = ss.str();
        return *this;
    }

    IniField &IniField::operator=(const float value)
    {
        std::stringstream ss;
        ss << value;
        value_ = ss.str();
        return *this;
    }

    IniField &IniField::operator=(const bool value)
    {
        if(value)
            value_ = "true";
        else
            value_ = "false";
        return *this;
    }

    IniField &IniField::operator=(const std::string &value)
    {
        value_ = value;
        return *this;
    }

    /************************************
     *          IniFile
     ************************************/

    IniFile::IniFile(const char fieldSep, const char comment)
        : fieldSep_(fieldSep), comment_(comment)
    {
    }

    IniFile::IniFile(const std::string &fileName, const char fieldSep,
                     const char comment)
        : fieldSep_(fieldSep), comment_(comment)
    {
        load(fileName);
    }

    IniFile::IniFile(std::istream &is, const char fieldSep, const char comment)
        : fieldSep_(fieldSep), comment_(comment)
    {
        decode(is);
    }

    IniFile::~IniFile()
    {
    }

    void IniFile::setFieldSep(const char sep)
    {
        fieldSep_ = sep;
    }

    void IniFile::setCommentChar(const char comment)
    {
        comment_ = comment;
    }

    void IniFile::decode(std::istream &is)
    {
        clear();
        int lineNo = 0;
        IniSection *currentSection = NULL;
        // iterate file by line
        while(!is.eof() && !is.fail()) {
            std::string line;
            std::getline(is, line, '\n');
            ++lineNo;

            // skip if line is empty
            if(line.size() == 0)
                continue;
            // skip if line is a comment
            if(line[0] == comment_)
                continue;
            if(line[0] == '[') {
                // line is a section
                // check if the section is also closed on same line
                std::size_t pos = line.find("]");
                if(pos == std::string::npos) {
                    std::stringstream ss;
                    ss << "l" << lineNo
                       << ": ini parsing failed, section not closed";
                    throw std::logic_error(ss.str());
                }
                // check if the section name is empty
                if(pos == 1) {
                    std::stringstream ss;
                    ss << "l" << lineNo
                       << ": ini parsing failed, section is empty";
                    throw std::logic_error(ss.str());
                }
                // check if there is a newline following closing bracket
                if(pos + 1 != line.length()) {
                    std::stringstream ss;
                    ss << "l" << lineNo
                       << ": ini parsing failed, no end of line after section";
                    throw std::logic_error(ss.str());
                }

                // retrieve section name
                std::string secName = line.substr(1, pos - 1);
                currentSection = &((*this)[secName]);
            } else {
                // line is a field definition
                // check if section was already opened
                if(currentSection == NULL) {
                    std::stringstream ss;
                    ss << "l" << lineNo
                       << ": ini parsing failed, field has no section";
                    throw std::logic_error(ss.str());
                }

                // find key value separator
                std::size_t pos = line.find(fieldSep_);
                if(pos == std::string::npos) {
                    std::stringstream ss;
                    ss << "l" << lineNo << ": ini parsing failed, no '=' found";
                    throw std::logic_error(ss.str());
                }
                // retrieve field name and value
                std::string name = line.substr(0, pos);
                std::string value = line.substr(pos + 1, std::string::npos);
                (*currentSection)[name] = value;
            }
        }
    }

    void IniFile::decode(const std::string &content)
    {
        std::istringstream ss(content);
        decode(ss);
    }

    void IniFile::load(const std::string &fileName)
    {
        std::ifstream is(fileName.c_str());
        decode(is);
    }

    void IniFile::encode(std::ostream &os)
    {
        IniFile::iterator it;
        // iterate through all sections in this file
        for(it = this->begin(); it != this->end(); it++) {
            os << "[" << it->first << "]" << std::endl;
            IniSection::iterator secIt;
            // iterate through all fields in the section
            for(secIt = it->second.begin(); secIt != it->second.end(); secIt++)
                os << secIt->first << fieldSep_ << secIt->second.asString()
                   << std::endl;
        }
    }

    std::string IniFile::encode()
    {
        std::ostringstream ss;
        encode(ss);
        return ss.str();
    }

    void IniFile::save(const std::string &fileName)
    {
        std::ofstream os(fileName.c_str());
        encode(os);
    }
}
