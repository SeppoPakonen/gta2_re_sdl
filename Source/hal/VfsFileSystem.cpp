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

    // Check GTA2_ROOT environment variable
    const char* gta2Root = std::getenv("GTA2_ROOT");
    if (gta2Root && *gta2Root) {
        AddSearchRoot(gta2Root);
        std::string rootData = std::string(gta2Root) + "/data";
        AddSearchRoot(rootData.c_str());
    }

#if !defined(_WIN32)
    // Check standard Wine default install paths
    const char* home = std::getenv("HOME");
    if (home && *home) {
        std::string wineDefault64 = std::string(home) + "/.wine/drive_c/Program Files (x86)/Rockstar Games/GTA2";
        std::string wineDefault32 = std::string(home) + "/.wine/drive_c/Program Files/Rockstar Games/GTA2";

        AddSearchRoot(wineDefault64.c_str());
        AddSearchRoot((wineDefault64 + "/data").c_str());
        AddSearchRoot(wineDefault32.c_str());
        AddSearchRoot((wineDefault32 + "/data").c_str());
    }
#endif

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

void VfsFileSystem::AddSearchRoot(const char* directoryPath) {
    std::error_code ec;
    if (directoryPath && fs::exists(directoryPath, ec)) {
        mSearchRoots.emplace_back(directoryPath);
    }
}

std::string VfsFileSystem::ResolvePath(const char* virtualPath) {
    std::string norm = NormalizePath(virtualPath ? virtualPath : "");
    fs::path target(norm);

    // Ordered read search list: user dir -> application (base) dir -> extra roots.
    std::vector<std::string> roots;
    roots.push_back(mUserDir);
    roots.push_back(mBaseDir);
    for (const auto& r : mSearchRoots) {
        roots.push_back(r);
    }

    std::string firstResolved;
    bool haveFirst = false;

    for (const auto& root : roots) {
        std::error_code ec;
        fs::path direct = fs::path(root) / target;
        if (fs::exists(direct, ec)) {
            return direct.string();
        }

        // Case-insensitive fallback traversal within this root.
        fs::path current = root;
        bool resolvedAll = true;
        for (const auto& part : target) {
            std::string partStr = part.string();
            bool found = false;
            std::error_code ec2;
            if (fs::exists(current, ec2) && fs::is_directory(current, ec2)) {
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
                resolvedAll = false;
                break;
            }
        }

        if (resolvedAll && fs::exists(current, ec)) {
            return current.string();
        }
        if (!haveFirst) {
            firstResolved = current.string();
            haveFirst = true;
        }
    }

    return haveFirst ? firstResolved : (fs::path(mBaseDir) / target).string();
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
    if (!mode) {
        mode = "rb";
    }

    bool isWrite = (std::strchr(mode, 'w') != nullptr || std::strchr(mode, 'a') != nullptr ||
                    std::strchr(mode, '+') != nullptr);

    std::string fullPath;
    if (isWrite) {
        // Writes always go to the writable user directory.
        fullPath = (fs::path(mUserDir) / NormalizePath(virtualPath ? virtualPath : "")).string();
        std::error_code ec;
        fs::create_directories(fs::path(fullPath).parent_path(), ec);
    } else {
        fullPath = ResolvePath(virtualPath);
    }

    std::FILE* fp = std::fopen(fullPath.c_str(), mode);
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
    // Writes always go to the writable user directory.
    std::string resolved = (fs::path(mUserDir) / NormalizePath(virtualPath ? virtualPath : "")).string();

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