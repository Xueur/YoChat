#pragma once
#include <fstream>  
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>  
#include <boost/filesystem.hpp>    
#include <map>
#include <iostream>

struct SectionInfo
{
    SectionInfo() = default;
    ~SectionInfo() { _section_datas.clear(); }
    SectionInfo(const SectionInfo& src) {
        _section_datas = src._section_datas;
    }
    SectionInfo& operator=(const SectionInfo& src) {
        if (this != &src) {
            _section_datas = src._section_datas;
        }
        return *this;
    }

    std::map<std::string, std::string> _section_datas;
    std::string operator[](const std::string& key) {
        if (_section_datas.find(key) != _section_datas.end()) {
            return _section_datas[key];
        }
        return "";
    }

};
class ConfigMgr
{
public:
    ~ConfigMgr()
    {
        _config_map.clear();
    }

    SectionInfo operator[](const std::string& section) {
        if (_config_map.find(section) != _config_map.end()) {
            return _config_map[section];
        }
        return SectionInfo();
    }
    static ConfigMgr& getInstance() {
        static ConfigMgr instance;
        return instance;
    }
    ConfigMgr(const ConfigMgr& src) {
        _config_map = src._config_map;
    }
    ConfigMgr& operator=(const ConfigMgr& src) {
        if (this != &src) {
            _config_map = src._config_map;
        }
        return *this;
    }
private:
    ConfigMgr();
    ConfigMgr(ConfigMgr&&) = delete;
    ConfigMgr& operator=(ConfigMgr&&) = delete;
    std::map<std::string, SectionInfo> _config_map;
};

