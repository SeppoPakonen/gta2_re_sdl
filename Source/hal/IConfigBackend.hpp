#pragma once
#include "HalTypes.hpp"
#include <string>

namespace gta2::hal {

class IConfigBackend {
public:
    virtual ~IConfigBackend() = default;

    virtual bool Load(const char* configFilePath) = 0;
    virtual bool Save(const char* configFilePath) = 0;

    virtual s32  GetInt(const char* section, const char* key, s32 defaultValue) = 0;
    virtual void SetInt(const char* section, const char* key, s32 value) = 0;

    virtual f32  GetFloat(const char* section, const char* key, f32 defaultValue) = 0;
    virtual void SetFloat(const char* section, const char* key, f32 value) = 0;

    virtual std::string GetString(const char* section, const char* key, const char* defaultValue) = 0;
    virtual void SetString(const char* section, const char* key, const char* value) = 0;

    virtual bool HasKey(const char* section, const char* key) = 0;
    virtual void DeleteKey(const char* section, const char* key) = 0;
};

} // namespace gta2::hal