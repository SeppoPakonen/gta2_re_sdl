#pragma once
#include "HalTypes.hpp"
#include <string>
#include <vector>

namespace gta2::hal {

class IFileStream {
public:
    virtual ~IFileStream() = default;
    virtual size_t Read(void* pDest, size_t size) = 0;
    virtual size_t Write(const void* pSrc, size_t size) = 0;
    virtual bool   Seek(s64 offset, s32 origin) = 0; // origin: 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END
    virtual s64    Tell() = 0;
    virtual s64    GetSize() = 0;
    virtual void   Close() = 0;
};

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    virtual bool Initialize(const char* baseDir, const char* userPrefDir) = 0;
    virtual void Shutdown() = 0;

    virtual bool FileExists(const char* virtualPath) = 0;
    virtual s64  GetFileSize(const char* virtualPath) = 0;

    virtual IFileStream* OpenFile(const char* virtualPath, const char* mode) = 0;
    virtual bool ReadEntireFile(const char* virtualPath, std::vector<u8>& outBuffer) = 0;
    virtual bool WriteEntireFile(const char* virtualPath, const void* pData, size_t size) = 0;

    // Case-insensitive path normalizer (translates "data\\nyc.gmp" -> "/path/to/data/NYC.GMP")
    virtual std::string ResolvePath(const char* virtualPath) = 0;
};

} // namespace gta2::hal