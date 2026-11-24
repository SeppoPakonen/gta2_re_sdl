#pragma once

#include "IFileSystem.hpp"
#include <cstdio>
#include <string>
#include <vector>

namespace gta2::hal {

// File stream backed by a stdio FILE*. Implements IFileStream.
class VfsFileStream : public IFileStream {
  public:
    VfsFileStream(std::FILE* fp, s64 size);
    ~VfsFileStream() override;

    size_t Read(void* pDest, size_t size) override;
    size_t Write(const void* pSrc, size_t size) override;
    bool Seek(s64 offset, s32 origin) override;
    s64 Tell() override;
    s64 GetSize() override;
    void Close() override;

  private:
    std::FILE* mFile;
    s64 mSize;
};

// Case-insensitive virtual file system. Translates legacy Windows-style
// backslash paths and resolves components against the real (case-sensitive)
// disk layout by scanning directories.
class VfsFileSystem : public IFileSystem {
  public:
    bool Initialize(const char* baseDir, const char* userPrefDir) override;
    void Shutdown() override;

    bool FileExists(const char* virtualPath) override;
    s64 GetFileSize(const char* virtualPath) override;

    IFileStream* OpenFile(const char* virtualPath, const char* mode) override;
    bool ReadEntireFile(const char* virtualPath, std::vector<u8>& outBuffer) override;
    bool WriteEntireFile(const char* virtualPath, const void* pData, size_t size) override;

    std::string ResolvePath(const char* virtualPath) override;

    // Register an additional read-only asset search root (e.g. system data dir).
    void AddSearchRoot(const char* directoryPath);

    // Convert backslashes to forward slashes and strip redundant leading "./".
    static std::string NormalizePath(std::string path);

  private:
    std::string mBaseDir;
    std::string mUserDir;
    std::vector<std::string> mSearchRoots;
};

} // namespace gta2::hal