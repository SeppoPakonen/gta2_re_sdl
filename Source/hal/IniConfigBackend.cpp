#include "IniConfigBackend.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>

namespace gta2::hal {

namespace {

std::string Trim(const std::string& s) {
    size_t begin = 0;
    while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) {
        ++begin;
    }
    size_t end = s.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(begin, end - begin);
}

} // namespace

bool IniConfigBackend::Load(const char* configFilePath) {
    mFilePath = configFilePath ? configFilePath : "";
    mSections.clear();

    std::ifstream in(mFilePath.c_str());
    if (!in) {
        return false;
    }

    std::string line;
    std::string section;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::string t = Trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';') {
            continue;
        }

        if (t.front() == '[' && t.back() == ']') {
            section = Trim(t.substr(1, t.size() - 2));
            mSections[section];
            continue;
        }

        std::string::size_type eq = t.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = Trim(t.substr(0, eq));
        std::string value = Trim(t.substr(eq + 1));
        mSections[section][key] = value;
    }

    return true;
}

bool IniConfigBackend::Save(const char* configFilePath) {
    if (configFilePath && *configFilePath) {
        mFilePath = configFilePath;
    }

    std::ofstream out(mFilePath.c_str());
    if (!out) {
        return false;
    }

    for (const auto& section : mSections) {
        out << "[" << section.first << "]\n";
        for (const auto& kv : section.second) {
            out << kv.first << "=" << kv.second << "\n";
        }
        out << "\n";
    }

    return true;
}

s32 IniConfigBackend::GetInt(const char* section, const char* key, s32 defaultValue) {
    auto s = mSections.find(section ? section : "");
    if (s == mSections.end()) {
        return defaultValue;
    }
    auto k = s->second.find(key ? key : "");
    if (k == s->second.end()) {
        return defaultValue;
    }
    try {
        return static_cast<s32>(std::stol(k->second));
    } catch (...) {
        return defaultValue;
    }
}

void IniConfigBackend::SetInt(const char* section, const char* key, s32 value) {
    mSections[section ? section : ""][key ? key : ""] = std::to_string(value);
}

f32 IniConfigBackend::GetFloat(const char* section, const char* key, f32 defaultValue) {
    auto s = mSections.find(section ? section : "");
    if (s == mSections.end()) {
        return defaultValue;
    }
    auto k = s->second.find(key ? key : "");
    if (k == s->second.end()) {
        return defaultValue;
    }
    try {
        return static_cast<f32>(std::stof(k->second));
    } catch (...) {
        return defaultValue;
    }
}

void IniConfigBackend::SetFloat(const char* section, const char* key, f32 value) {
    mSections[section ? section : ""][key ? key : ""] = std::to_string(value);
}

std::string IniConfigBackend::GetString(const char* section, const char* key, const char* defaultValue) {
    auto s = mSections.find(section ? section : "");
    if (s == mSections.end()) {
        return defaultValue ? defaultValue : "";
    }
    auto k = s->second.find(key ? key : "");
    if (k == s->second.end()) {
        return defaultValue ? defaultValue : "";
    }
    return k->second;
}

void IniConfigBackend::SetString(const char* section, const char* key, const char* value) {
    mSections[section ? section : ""][key ? key : ""] = value ? value : "";
}

bool IniConfigBackend::HasKey(const char* section, const char* key) {
    auto s = mSections.find(section ? section : "");
    if (s == mSections.end()) {
        return false;
    }
    return s->second.find(key ? key : "") != s->second.end();
}

void IniConfigBackend::DeleteKey(const char* section, const char* key) {
    auto s = mSections.find(section ? section : "");
    if (s == mSections.end()) {
        return;
    }
    s->second.erase(key ? key : "");
}

} // namespace gta2::hal