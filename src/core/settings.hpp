#pragma once 

#include <fstream>
#include <string>
#include <map>


enum ValueType 
{
    ValueType_Float,
    ValueType_Uint,
    ValueType_Bool,
    ValueType_Int
};

class Setting
{
    public:

    const bool isDirectory;
    const bool isValue;

    virtual const float& as_float() const = 0;
    virtual float& as_float()             = 0;

    virtual const unsigned int& as_uint() const = 0;
    virtual unsigned int& as_uint()             = 0;

    virtual const int& as_int() const = 0;
    virtual int& as_int()             = 0;

    virtual const bool& as_bool() const = 0;
    virtual bool& as_bool()             = 0;

    virtual ~Setting() = default;

    protected:

    Setting(bool dir, bool val) : isDirectory(dir), isValue(val) {}
};

class SettingValue : public Setting
{
    public:

    SettingValue(ValueType type) : Setting(false, true) , _type(type) {}

    const float& as_float() const override { return _value.as_float; }
    float& as_float()             override { return _value.as_float; }
    operator float&()        { return _value.as_float; }
    operator float()        const { return _value.as_float; }

    const unsigned int& as_uint() const override { return _value.as_uint; }
    unsigned int& as_uint()             override { return _value.as_uint; }
    operator unsigned int&() { return _value.as_uint;  }
    operator unsigned int() const { return _value.as_uint;  }

    const int& as_int() const override { return _value.as_int; }
    int& as_int()             override { return _value.as_int; }
    operator int&()          { return _value.as_int;   }
    operator int()          const { return _value.as_int;   }


    const bool& as_bool() const override { return _value.as_bool; }
    bool& as_bool()             override { return _value.as_bool; }
    operator bool&()         { return _value.as_bool;  }
    operator bool()         const { return _value.as_bool;  }


    ValueType get_type() const { return _type; }

    private:

    union 
    {
        float as_float;
        unsigned int as_uint;
        int as_int;
        bool as_bool;
    } _value;
    
    ValueType _type;
};


class SettingsDirectory : public Setting 
{
    friend class SettingsManager;

    public:

    SettingsDirectory() : Setting(true, false) {}

    Setting *lookup(const std::string& path);

    const Setting* lookup(const std::string& path) const;

    SettingValue *set(const std::string& path, float value);
    
    SettingValue *set(const std::string& path, unsigned int value);
    
    SettingValue *set(const std::string& path, bool value);

    SettingValue *set(const std::string& path, int value);

    const float& as_float() const {};
    float& as_float()             {};

    const unsigned int& as_uint() const {};
    unsigned int& as_uint()             {};

    const int& as_int() const {};
    int& as_int()             {};

    const bool& as_bool() const {};
    bool& as_bool()             {};

    private:

    void save(std::fstream& file, int depth) const;
    
    Setting *_get(const std::string& p);

    SettingValue *_get_or_create_value(const std::string &p, ValueType type);

    SettingsDirectory *_get_or_create_dir(const std::string &p);

    const Setting *_get(const std::string& p) const;

    std::map<std::string, Setting*> _contents;
};

class SettingsManager
{
    public:

    static SettingsManager& get()
    {
        static SettingsManager settings = SettingsManager();
        return settings;
    }

    Setting& operator[](const std::string& v)
    {
        return *root->lookup(v);
    }

    SettingValue* set(const std::string &k, float val){
        return root->set(k, val);
    }
    SettingValue* set(const std::string &k, bool val){
        return root->set(k, val);
    }
    SettingValue* set(const std::string &k, unsigned int val){
        return root->set(k, val);
    }
    SettingValue* set(const std::string &k, int val){
        return root->set(k, val);
    }

    bool load(const char* path);

    void save(const char* path) const;

    private:

    SettingsManager() {
        root = new SettingsDirectory();
    }
    
    SettingsDirectory *root;

};