#ifndef INIFILE_HPP_
#define INIFILE_HPP_

#include <istream>
#include <map>

namespace inifile
{

    class IniField
    {
    private:
        std::string value_;
    public:
        IniField();
        IniField(const std::string &value);
        IniField(const IniField &field);
        ~IniField();

        const std::string& asString() const;
        int asInt() const;
        double asDouble() const;
        bool asBool() const;

        IniField& operator=(const IniField& field);
        IniField& operator=(const int value);
        IniField& operator=(const double value);
        IniField& operator=(const bool value);
        IniField& operator=(const std::string &value);
    };

    class IniSection
    {
    public:
        IniSection();
        ~IniSection();

        IniField& operator[](const std::string &fieldName);

        typedef std::map<std::string, IniField>::iterator iterator;
        typedef std::map<std::string, IniField>::const_iterator const_iterator;
        iterator begin();
        const_iterator begin();
        iterator end();
        const_iterator end();
    };

    class IniFile
    {
    private:
        std::map <std::string, IniSection> sections_;
    public:
        IniFile();
        ~IniFile();

        void load(const std::istream &is);
        void load(const std::string &fileName);

        void save(std::ostream &os);
        void save(const std::string &fileName);

        IniSection& operator[](const std::string &sectionName);

        typedef std::map<std::string, IniField>::iterator iterator;
        typedef std::map<std::string, IniField>::const_iterator const_iterator;
        iterator begin();
        const_iterator begin();
        iterator end();
        const_iterator end();
    };
}

#endif
