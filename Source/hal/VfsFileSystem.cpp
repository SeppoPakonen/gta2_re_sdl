#include "VfsFileSystem.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <system_error>

#if defined(_WIN32)
    #define strcasecmp _stricmp
#else
    #include <strings.h>
#endif

namespace fs = std::filesystem;

namespace gta2::hal {

VfsFileStream::VfsFileStream(std::FILE* fp, s64 size) : mFile(fp), mSize(size) {}

VfsFileStream::~VfsFileStream() {
    Close();
}

size_t VfsFileStream::Read(void* pDest, size_t size) {
    if (!mFile) {
        return 0;
    }
    return std::fread(pDest, 1, size, mFile);
}

size_t VfsFileStream::Write(const void* pSrc, size_t size) {
    if (!mFile) {
        return 0;
    }
    return std::fwrite(pSrc, 1, size, mFile);
}

bool VfsFileStream::Seek(s64 offset, s32 origin) {
    if (!mFile) {
        return false;
    }
    return std::fseek(mFile, static_cast<long>(offset), origin) == 0;
}

s64 VfsFileStream::Tell() {
    if (!mFile) {
        return -1;
    }
    return static_cast<s64>(std::ftell(mFile));
}

s64 VfsFileStream::GetSize() {
    return mSize;
}

void VfsFileStream::Close() {
    if (mFile) {
        std::fclose(mFile);
        mFile = nullptr;
    }
}

bool VfsFileSystem::Initialize(const char* baseDir, const char* userPrefDir) {
    mBaseDir = baseDir ? baseDir : ".";
    mUserDir = userPrefDir ? userPrefDir : mBaseDir;
    return true;
}

void VfsFileSystem::Shutdown() {}

std::string VfsFileSystem::NormalizePath(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    // Remove redundant leading slashes
    while (path.rfind("./", 0) == 0) {
        path = path.substr(2);
    }
    return path;
}

std::string VfsFileSystem::ResolvePath(const char* virtualPath) {
    std::string norm = NormalizePath(virtualPath ? virtualPath : "");
    fs::path target(norm);

    // Check user directory first (for modified configs / saves)
    fs::path userPath = fs::path(mUserDir) / target;
    if (fs::exists(userPath)) {
        return userPath.string();
    }

    // Check base directory
    fs::path basePath = fs::path(mBaseDir) / target;
    if (fs::exists(basePath)) {
        return basePath.string();
    }

    // Case-insensitive fallback traversal
    fs::path current = mBaseDir;
    for (const auto& part : target) {
        std::string partStr = part.string();
        bool found = false;
        std::error_code ec;
        if (fs::exists(current, ec) && fs::is_directory(current, ec)) {
            for (const auto& entry : fs::directory_iterator(current)) {
                std::string entryName = entry.path().filename().string();
                if (strcasecmp(entryName.c_str(), partStr.c_str()) == 0) {
                    current /= entryName;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            current /= partStr;
        }
    }
    return current.string();
}

bool VfsFileSystem::FileExists(const char* virtualPath) {
    std::error_code ec;
    return fs::exists(fs::path(ResolvePath(virtualPath)), ec);
}

s64 VfsFileSystem::GetFileSize(const char* virtualPath) {
    std::error_code ec;
    s64 size = fs::file_size(fs::path(ResolvePath(virtualPath)), ec);
    return ec ? -1 : size;
}

IFileStream* VfsFileSystem::OpenFile(const char* virtualPath, const char* mode) {
    std::string resolved = ResolvePath(virtualPath);
    std::FILE* fp = std::fopen(resolved.c_str(), mode ? mode : "rb");
    if (!fp) {
        return nullptr;
    }

    s64 size = 0;
    if (std::fseek(fp, 0, SEEK_END) == 0) {
        size = static_cast<s64>(std::ftell(fp));
        std::fseek(fp, 0, SEEK_SET);
    }
    return new VfsFileStream(fp, size);
}

bool VfsFileSystem::ReadEntireFile(const char* virtualPath, std::vector<u8>& outBuffer) {
    outBuffer.clear();

    IFileStream* stream = OpenFile(virtualPath, "rb");
    if (!stream) {
        return false;
    }

    s64 size = stream->GetSize();
    if (size > 0) {
        outBuffer.resize(static_cast<size_t>(size));
        size_t read = stream->Read(outBuffer.data(), outBuffer.size());
        outBuffer.resize(read);
    }

    stream->Close();
    delete stream;
    return true;
}

bool VfsFileSystem::WriteEntireFile(const char* virtualPath, const void* pData, size_t size) {
    std::string resolved = ResolvePath(virtualPath);

    // Ensure the parent directory exists.
    std::error_code ec;
    fs::path parent = fs::path(resolved).parent_path();
    if (!parent.empty()) {
        fs::create_directories(parent, ec);
    }

    std::FILE* fp = std::fopen(resolved.c_str(), "wb");
    if (!fp) {
        return false;
    }

    size_t written = size ? std::fwrite(pData, 1, size, fp) : 0;
    std::fclose(fp);
    return written == size;
}

} // namespace gta2::hal