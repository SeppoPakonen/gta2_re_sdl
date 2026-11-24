#pragma once

#include "IConfigBackend.hpp"
#include <map>
#include <string>

namespace gta2::hal {

class IniConfigBackend : public IConfigBackend {
  private:
    std::string mFilePath;
    std::map<std::string, std::map<std::string, std::string>> mSections;

  public:
    bool Load(const char* configFilePath) override;
    bool Save(const char* configFilePath) override;

    s32 GetInt(const char* section, const char* key, s32 defaultValue) override;
    void SetInt(const char* section, const char* key, s32 value) override;

    f32 GetFloat(const char* section, const char* key, f32 defaultValue) override;
    void SetFloat(const char* section, const char* key, f32 value) override;

    std::string GetString(const char* section, const char* key, const char* defaultValue) override;
    void SetString(const char* section, const char* key, const char* value) override;

    bool HasKey(const char* section, const char* key) override;
    void DeleteKey(const char* section, const char* key) override;
};

} // namespace gta2::hal