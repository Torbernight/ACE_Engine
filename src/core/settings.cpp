#include <iostream>
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

SettingValue* SettingsDirectory::set(const std::string& path, int value)
{
    size_t slash = path.find_first_of('/');
    if (slash == std::string::npos)
    {
        SettingValue* val = _get_or_create_value(path, ValueType_Int);
        if (val == nullptr){
            return nullptr;
        }
        val->as_int() = value;
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

void SettingsDirectory::save(std::fstream& outFile, int depth) const
{
    std::string indent;
    for(int i = 0; i < depth; i++){
        indent += ' ';
    }
    //(' ', depth);

    for (const auto& kv : _contents)
    {
        const std::string& key = kv.first;
        const Setting* val = kv.second;

        outFile << indent << key << ": ";

        if (val->isValue)
        {
            const SettingValue *value = static_cast<const SettingValue*>(val);
            switch(value->get_type())
            {
                case ValueType_Float:
                    outFile << value->as_float();
                    break;
                case ValueType_Uint:
                    outFile << value->as_uint();
                    break;
                case ValueType_Bool:
                    outFile << value->as_bool();
                    break;
                case ValueType_Int:
                    outFile << value->as_int();
                    break;
            }
            outFile << "\n";
        } else {
            const SettingsDirectory *dir = static_cast<const SettingsDirectory*>(val);
            outFile << "\n";
            dir->save(outFile, depth+1);
        }
    }
}

#include <vector>
#include <bits/stdc++.h>

bool SettingsManager::load(const char* path)
{
    std::fstream inFile(path, std::ios_base::in);
    std::vector<std::string> stem;
    char key[128];
    char val[128];
    int lastLevel = 0;
    int level = 0;
    while(inFile.get() != -1)
    {
        inFile.unget();
        level = 0;
        while(inFile.get() == ' '){
            level++;
        }
        inFile.unget();

        if (lastLevel > level){
            stem.pop_back();
        }

        inFile.getline(key, 128, ':');
        inFile.seekg(1, std::ios_base::cur); // whitespace
        inFile.getline(val, 128);

        if (val[0] == '\0') // no value, should be directory
        {
            stem.push_back(key);
            level++;
        } else {
            ValueType type = ValueType_Int;

            for(int i = 0; i < 128; i++)
            {
                if(val[i] == '\0'){
                    break;
                }
                if (val[i] == '.'){
                    type = ValueType_Float;
                    break;
                }
                if (val[i] == '-'){
                    type = ValueType_Int;
                    break;
                }
            }
            std::string fullKey = "";
            for(int j = 0; j < stem.size(); j++){
                fullKey += stem[j];
            }
            fullKey += key;
            if (type == ValueType_Float)
            {
                this->set(fullKey, std::stof(val));
            } else {
                this->set(fullKey, std::stoi(val));
            }
        }
        lastLevel = level;
    }

    return true;
}

void SettingsManager::save(const char* path) const
{
    std::fstream outFile(path, std::ios_base::out);
    this->root->save(outFile, 0);
    
}
