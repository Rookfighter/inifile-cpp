#include "IniFile.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <fstream>

#define TO_UPPER(str) std::transform(str.begin(), str.end(),str.begin(), ::toupper)

namespace inifile
{
    static bool strToBool (const std::string &value)
    {
        std::string tmp (value);
        TO_UPPER(tmp);

        if(tmp == "TRUE")
            return true;
        else if(tmp == "FALSE")
            return false;
        else
            throw std::domain_error("field is not a bool");
    }

    static int strToInt (const std::string &value)
    {
        char *endptr;
        int result = strtol(value.c_str(), &endptr, 10);
        if (*endptr != '\0')
            throw std::domain_error("field is not an int");

        return result;
    }

    static double strToDouble (const std::string &value)
    {
        char *endptr;
        double result = strtod(value.c_str(), &endptr);
        if (*endptr != '\0')
            throw std::domain_error("field is not a double");

        return result;
    }

    IniFile::IniFile()
    {
    }

    IniFile::~IniFile()
    {
    }

    void IniFile::load(std::istream& is)
    {
        clear();
        int lineNo = 0;
        std::string currentSection;
        while (!is.eof() && is.fail())
        {
            std::string line;
            std::getline(is, line, '\n');
            ++lineNo;

            if(line.size() == 0)
                continue;
            if (line[0] == '#')
                continue;
            if (line[0] == '[')
            {
            }
            else
            {
                std::size_t pos = line.find("=");
                if (pos == std::string::npos)
                {
                    std::stringstream ss;
                    ss << "l" << lineNo << ": parsing failed, no '=' found";
                    throw std::logic_error(ss.str());
                }
                std::string name = line.substr(0, pos);
                std::string value = line.substr(pos + 1, std::string::npos);
                (*this)[currentSection][name] = value;
            }
        }
    }

    void IniFile::load(const std::string& fileName)
    {
        std::ifstream is(fileName);
        load(is);
    }

    void IniFile::save(std::ostream& os)
    {
    }

    void IniFile::save(const std::string& fileName)
    {
        std::ofstream os(fileName);
        save(os);
    }

    IniField::IniField()
    {
    }

    IniField::IniField(const std::string &value)
    :value_(value)
    {

    }

    IniField::IniField(const IniField& field)
    :value_(field.value_)
    {
    }

    IniField::~IniField()
    {
    }

    const std::string& IniField::asString() const
    {
        return value_;
    }

    int IniField::asInt() const
    {
        return strToInt(value_);
    }

    double IniField::asDouble() const
    {
        return strToDouble(value_);
    }

    bool IniField::asBool() const
    {
        return strToBool(value_);
    }

    IniField& IniField::operator=(const IniField& field)
    {
        value_ = field.value_;
        return *this;
    }

    IniField& IniField::operator=(const int value)
    {
        std::stringstream ss;
        ss << value;
        value_ = ss.str();
        return *this;
    }

    IniField& IniField::operator=(const double value)
    {
        std::stringstream ss;
        ss << value;
        value_ = ss.str();
        return *this;
    }

    IniField& IniField::operator=(const bool value)
    {
        if (value)
            value_ = "true";
        else
            value_ = "false";
        return *this;
    }

    IniField& IniField::operator=(const std::string &value)
    {
        value_ = value;
        return *this;
    }

    IniSection::IniSection()
    {
    }

    IniSection::~IniSection()
    {
    }
}
