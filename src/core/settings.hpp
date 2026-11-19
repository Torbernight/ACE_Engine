#pragma once 

#include <string>
#include <map>

class SettingsManager 
{
    public:

    static SettingsManager& get()
    {
        static SettingsManager manager = SettingsManager();
        return manager;
    }

    float get_float(const std::string& param) const;
    void set_float(const std::string& param, float value);

    bool get_bool(const std::string& param) const;
    void set_bool(const std::string& param, bool value);

    unsigned int get_uint(const std::string& param) const;
    void set_uint(const std::string& param, bool value);

    private:

    SettingsManager() {}

    struct {
        std::map<std::string, float> f_params;
        std::map<std::string, unsigned int > uint_params;
        std::map<std::string, bool > bool_params;
    } m;


};