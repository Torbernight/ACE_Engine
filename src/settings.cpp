#include "settings.hpp"



float SettingsManager::get_float(const std::string& param) const
{
    if (m.f_params.count(param) == 0)
    {
        // TODO: log system aksjdaskjd

    }
    return m.f_params.at(param);
}

void SettingsManager::set_float(const std::string& param, float value)
{
    m.f_params[param] = value;
}

bool SettingsManager::get_bool(const std::string& param) const
{
    if (m.bool_params.count(param) == 0)
    {

    }
    return m.bool_params.at(param);

}
void SettingsManager::set_bool(const std::string& param, bool value)
{
    m.bool_params[param] = value;
}

unsigned int SettingsManager::get_uint(const std::string& param) const
{
    if (m.uint_params.count(param) == 0)
    {

    }
    return m.uint_params.at(param);
}
void SettingsManager::set_uint(const std::string& param, bool value)
{
    m.uint_params[param] = value;
}
