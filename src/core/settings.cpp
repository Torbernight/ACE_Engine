#include "settings.hpp"

Setting* SettingsDirectory::lookup(const std::string& path)
{
    size_t slash = path.find_first_of('/');
    
    if (slash == std::string::npos)
    {
        return _get(path);
    } else {
        std::string root = path.substr(0, slash);
        Setting *setting = _get(root);
        if (setting == nullptr){
            // directory not found
            return nullptr;
        }
        SettingsDirectory *newDir = dynamic_cast<SettingsDirectory*>(setting);
        if (newDir == nullptr){
            // not a directory
            return nullptr;
        }

        std::string rem  = path.substr(slash+1);

        return newDir->lookup(rem);
    }
}

const Setting* SettingsDirectory::lookup(const std::string& path) const 
{
    size_t slash = path.find_first_of('/');
    
    if (slash == std::string::npos)
    {
        return _get(path);
    } else {
        std::string root = path.substr(0, slash);
        const Setting *setting = _get(root);
        if (setting == nullptr){
            // directory not found
            return nullptr;
        }
        const SettingsDirectory *newDir = dynamic_cast<const SettingsDirectory*>(setting);
        if (newDir == nullptr){
            // not a directory
            return nullptr;
        }

        std::string rem  = path.substr(slash+1);

        return newDir->lookup(rem);
    }
}

SettingValue* SettingsDirectory::set(const std::string& path, float value)
{
    size_t slash = path.find_first_of('/');
    if (slash == std::string::npos)
    {
        SettingValue* val = _get_or_create_value(path, ValueType_Float);
        if (val == nullptr){
            return nullptr;
        }
        val->as_float() = value;
        return val;
    } else {
        std::string root = path.substr(0, slash);

        SettingsDirectory *dir = _get_or_create_dir(root);
        if (dir == nullptr){
            return nullptr;
        }
        std::string rem  = path.substr(slash+1);
        return dir->set(rem, value);
    }
}

SettingValue* SettingsDirectory::set(const std::string& path, unsigned int value)
{
    size_t slash = path.find_first_of('/');
    if (slash == std::string::npos)
    {
        SettingValue* val = _get_or_create_value(path, ValueType_Uint);
        if (val == nullptr){
            return nullptr;
        }
        val->as_uint() = value;
        return val;
    } else {
        std::string root = path.substr(0, slash);

        SettingsDirectory *dir = _get_or_create_dir(root);
        if (dir == nullptr){
            return nullptr;
        }
        std::string rem  = path.substr(slash+1);
        return dir->set(rem, value);
    }
}

SettingValue* SettingsDirectory::set(const std::string& path, bool value)
{
    size_t slash = path.find_first_of('/');
    if (slash == std::string::npos)
    {
        SettingValue* val = _get_or_create_value(path, ValueType_Bool);
        if (val == nullptr){
            return nullptr;
        }
        val->as_bool() = value;
        return val;
    } else {
        std::string root = path.substr(0, slash);

        SettingsDirectory *dir = _get_or_create_dir(root);
        if (dir == nullptr){
            return nullptr;
        }
        std::string rem  = path.substr(slash+1);
        return dir->set(rem, value);
    }
}

Setting* SettingsDirectory::_get(const std::string& p)
{
    if (_contents.count(p) == 0){
        // setting not found
        return nullptr;
    } else {
        return _contents[p];
    }
}

const Setting* SettingsDirectory::_get(const std::string& p) const
{
    if (_contents.count(p) == 0){
        // setting not found
        return nullptr;
    } else {
        return _contents.at(p);
    }
}

SettingValue* SettingsDirectory::_get_or_create_value(const std::string &p, ValueType type)
{
    if (_contents.count(p) == 0){
        _contents[p] = (Setting*) new SettingValue(type);
    } else if (_contents[p]->isDirectory){
        // can't create value, there's a directory
        return nullptr;
    }
    return dynamic_cast<SettingValue*>(_contents[p]);
}

SettingsDirectory *SettingsDirectory::_get_or_create_dir(const std::string &p)
{
    if (_contents.count(p) == 0){
        _contents[p] = (Setting*) new SettingsDirectory();
    } else if (_contents[p]->isValue){
        // can't create value, there's a value
        return nullptr;
    }
    return dynamic_cast<SettingsDirectory*>(_contents[p]);
}

