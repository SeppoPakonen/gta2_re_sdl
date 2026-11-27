#pragma once

#if defined(GTA2_MODERN_BUILD)
    #include <cstdint>
    #include <cstddef>
    #include <cstdarg>
    #include <cstdio>
#endif

// Verbose runtime diagnostics. Off by default; enable with the GTA2_VERBOSE
// environment variable. In the matching (VC6) build the macro expands to the
// original plain printf so codegen is unchanged; only the modern build gates it.
namespace gta2 { extern bool gVerboseLog; }
#if defined(GTA2_MODERN_BUILD) || defined(GTA2_PLATFORM_LINUX) || defined(GTA2_USE_SDL2) || defined(GTA2_USE_SDL3)
    #define GTA2_VLOG(...) do { if (gta2::gVerboseLog) { printf(__VA_ARGS__); } } while (0)
#else
    #define GTA2_VLOG(...) printf(__VA_ARGS__)
#endif

typedef unsigned char u8;
typedef signed char s8;
typedef char char_type;

typedef unsigned short int u16;
typedef signed short int s16;

typedef unsigned int u32;
typedef signed int s32;

typedef float f32;
typedef double f64;

#if defined(GTA2_MODERN_BUILD)
    typedef uint64_t u64;
    typedef int64_t  s64;
    #define GTA2_PTR_TO_S32(p) static_cast<s32>(reinterpret_cast<uintptr_t>(p))
    #define GTA2_PTR_TO_U32(p) static_cast<u32>(reinterpret_cast<uintptr_t>(p))
#else
    #define GTA2_PTR_TO_S32(p) reinterpret_cast<s32>(p)
    #define GTA2_PTR_TO_U32(p) reinterpret_cast<u32>(p)
#endif

// Endianness abstraction. File formats (.GMP/.STY/.SCR/.GXT) are little-endian;
// these helpers convert between little-endian on-disk order and host order.
#if defined(GTA2_MODERN_BUILD)
    #if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
        #include <sys/endian.h>
    #elif defined(__linux__) || defined(__ANDROID__)
        #include <endian.h>
    #endif

    #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        #define GTA2_BIG_ENDIAN 1
        #define gta2_le16toh(x) __builtin_bswap16(x)
        #define gta2_le32toh(x) __builtin_bswap32(x)
        #define gta2_htole16(x) __builtin_bswap16(x)
        #define gta2_htole32(x) __builtin_bswap32(x)
    #else
        #define GTA2_LITTLE_ENDIAN 1
        #define gta2_le16toh(x) (x)
        #define gta2_le32toh(x) (x)
        #define gta2_htole16(x) (x)
        #define gta2_htole32(x) (x)
    #endif
#endif

// Compiler-specific shims so modern (non-MSVC) compilers accept the legacy
// MSVC spellings used throughout the sources. Only active in the modern build.
#if defined(GTA2_MODERN_BUILD) && !defined(_MSC_VER)
    #ifndef __int64
        #define __int64 long long
    #endif
    #ifndef __forceinline
        #define __forceinline inline
    #endif
    #ifndef __cdecl
        #define __cdecl
    #endif
    #ifndef __fastcall
        #define __fastcall
    #endif
    #ifndef __declspec
        #define __declspec(x)
    #endif
#include <cwchar>
#include <wchar.h>
#include <cstdio>
#include <cstdarg>

#if !defined(_WIN32)
    inline size_t gta2_wcslen(const wchar_t* s) {
        if (!s) return 0;
        const wchar_t* p = s;
        while (*p) p++;
        return p - s;
    }
    inline wchar_t* gta2_wcscpy(wchar_t* dest, const wchar_t* src) {
        if (!dest || !src) return dest;
        wchar_t* d = dest;
        while ((*d++ = *src++));
        return dest;
    }
    inline wchar_t* gta2_wcsncpy(wchar_t* dest, const wchar_t* src, size_t n) {
        if (!dest || !src) return dest;
        size_t i = 0;
        for (; i < n && src[i]; i++) dest[i] = src[i];
        for (; i < n; i++) dest[i] = 0;
        return dest;
    }
    inline int gta2_wcscmp(const wchar_t* s1, const wchar_t* s2) {
        if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
        while (*s1 && (*s1 == *s2)) { s1++; s2++; }
        return static_cast<int>(*s1) - static_cast<int>(*s2);
    }
    inline int gta2_wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n) {
        if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
        while (n && *s1 && (*s1 == *s2)) { s1++; s2++; n--; }
        if (n == 0) return 0;
        return static_cast<int>(*s1) - static_cast<int>(*s2);
    }

    #define wcslen gta2_wcslen
    #define wcscpy gta2_wcscpy
    #define wcsncpy gta2_wcsncpy
    #define wcscmp gta2_wcscmp
    #define wcsncmp gta2_wcsncmp

    inline int gta2_vswprintf(wchar_t* buf, size_t maxLen, const wchar_t* fmt, va_list args) {
        if (!buf || maxLen == 0 || !fmt) return 0;
        size_t out = 0;
        while (*fmt && out + 1 < maxLen) {
            if (*fmt != L'%') {
                buf[out++] = *fmt++;
                continue;
            }
            fmt++; // skip %
            if (*fmt == L'%') {
                buf[out++] = L'%';
                fmt++;
                continue;
            }
            char spec[32] = "%";
            int si = 1;
            while (*fmt && (*fmt == L'-' || *fmt == L'+' || *fmt == L'0' || *fmt == L' ' || *fmt == L'#' ||
                            (*fmt >= L'0' && *fmt <= L'9') || *fmt == L'.' || *fmt == L'l' || *fmt == L'h')) {
                if (si < 30) spec[si++] = static_cast<char>(*fmt);
                fmt++;
            }
            wchar_t type = *fmt ? *fmt++ : L'\0';
            if (type == L's') {
                const wchar_t* s = va_arg(args, const wchar_t*);
                if (!s) s = L"(null)";
                while (*s && out + 1 < maxLen) {
                    buf[out++] = *s++;
                }
            } else if (type == L'S') {
                const char* s = va_arg(args, const char*);
                if (!s) s = "(null)";
                while (*s && out + 1 < maxLen) {
                    buf[out++] = static_cast<wchar_t>(*s++);
                }
            } else if (type == L'c') {
                int c = va_arg(args, int);
                buf[out++] = static_cast<wchar_t>(c);
            } else if (type == L'd' || type == L'i') {
                spec[si++] = 'd';
                spec[si] = 0;
                int val = va_arg(args, int);
                char tmp[64];
                snprintf(tmp, sizeof(tmp), spec, val);
                for (char* p = tmp; *p && out + 1 < maxLen; ++p) {
                    buf[out++] = static_cast<wchar_t>(*p);
                }
            } else if (type == L'u' || type == L'x' || type == L'X' || type == L'o') {
                spec[si++] = static_cast<char>(type);
                spec[si] = 0;
                unsigned int val = va_arg(args, unsigned int);
                char tmp[64];
                snprintf(tmp, sizeof(tmp), spec, val);
                for (char* p = tmp; *p && out + 1 < maxLen; ++p) {
                    buf[out++] = static_cast<wchar_t>(*p);
                }
            } else if (type == L'f') {
                spec[si++] = 'f';
                spec[si] = 0;
                double val = va_arg(args, double);
                char tmp[64];
                snprintf(tmp, sizeof(tmp), spec, val);
                for (char* p = tmp; *p && out + 1 < maxLen; ++p) {
                    buf[out++] = static_cast<wchar_t>(*p);
                }
            }
        }
        buf[out] = 0;
        return static_cast<int>(out);
    }

    inline int gta2_swprintf(wchar_t* buf, const wchar_t* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int ret = gta2_vswprintf(buf, 1024, fmt, args);
        va_end(args);
        return ret;
    }

    #ifndef swprintf
        #define swprintf gta2_swprintf
    #endif
    #ifndef vswprintf
        #define vswprintf(buf, fmt, args) gta2_vswprintf(buf, 1024, fmt, args)
    #endif
#endif
    #ifndef wsprintfA
        #include <cstdio>
        #define wsprintfA sprintf
    #endif
    #if !defined(_WIN32)
        #ifndef _itoa
            inline char* _itoa(int val, char* buf, int radix) {
                if (radix == 16) sprintf(buf, "%x", val);
                else if (radix == 8) sprintf(buf, "%o", val);
                else sprintf(buf, "%d", val);
                return buf;
            }
        #endif
        #ifndef _itow
            inline wchar_t* _itow(int val, wchar_t* buf, int radix) {
                if (radix == 16) swprintf(buf, L"%x", val);
                else if (radix == 8) swprintf(buf, L"%o", val);
                else swprintf(buf, L"%d", val);
                return buf;
            }
        #endif
    #endif
    #ifndef _strnicmp
        #define _strnicmp strncasecmp
    #endif
    #ifndef _stricmp
        #define _stricmp strcasecmp
    #endif
    #ifndef _stdcall
        #define _stdcall __stdcall
    #endif
    #if !defined(_WIN32)
        typedef unsigned int UINT;
        typedef uintptr_t UINT_PTR;
        typedef void* LPVOID;
    #endif

#if !defined(_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <string.h>

struct _finddata_t {
    unsigned int attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    size_t size;
    char name[260];
};

struct ModernFindHandle {
    DIR* dir;
    char pattern[260];
    char dirpath[260];
};

inline long _findfirst(const char* pattern, _finddata_t* data) {
    if (!pattern || !data) return -1;
    char pat_norm[260];
    strncpy(pat_norm, pattern, sizeof(pat_norm));
    pat_norm[sizeof(pat_norm) - 1] = 0;
    for (char* p = pat_norm; *p; ++p) {
        if (*p == '\\') *p = '/';
    }
    if (strchr(pat_norm, '*') == nullptr && strchr(pat_norm, '?') == nullptr) {
        struct stat st;
        if (stat(pat_norm, &st) == 0) {
            data->size = st.st_size;
            const char* slash = strrchr(pat_norm, '/');
            strncpy(data->name, slash ? slash + 1 : pat_norm, sizeof(data->name));
            data->name[sizeof(data->name) - 1] = 0;
            return 1;
        }
        return -1;
    }
    char dirpath[260] = ".";
    const char* slash = strrchr(pat_norm, '/');
    char filepattern[260];
    if (slash) {
        size_t dirlen = slash - pat_norm;
        strncpy(dirpath, pat_norm, dirlen);
        dirpath[dirlen] = 0;
        strncpy(filepattern, slash + 1, sizeof(filepattern));
    } else {
        strncpy(filepattern, pat_norm, sizeof(filepattern));
    }
    filepattern[sizeof(filepattern) - 1] = 0;
    DIR* d = opendir(dirpath);
    if (!d) return -1;
    struct dirent* de;
    while ((de = readdir(d)) != nullptr) {
        if (fnmatch(filepattern, de->d_name, FNM_CASEFOLD) == 0) {
            strncpy(data->name, de->d_name, sizeof(data->name));
            data->name[sizeof(data->name) - 1] = 0;
            ModernFindHandle* h = new ModernFindHandle{d, "", ""};
            strncpy(h->pattern, filepattern, sizeof(h->pattern));
            strncpy(h->dirpath, dirpath, sizeof(h->dirpath));
            return reinterpret_cast<long>(h);
        }
    }
    closedir(d);
    return -1;
}

inline int _findnext(long handle, _finddata_t* data) {
    if (handle <= 1 || !data) return -1;
    ModernFindHandle* h = reinterpret_cast<ModernFindHandle*>(handle);
    struct dirent* de;
    while ((de = readdir(h->dir)) != nullptr) {
        if (fnmatch(h->pattern, de->d_name, FNM_CASEFOLD) == 0) {
            strncpy(data->name, de->d_name, sizeof(data->name));
            data->name[sizeof(data->name) - 1] = 0;
            return 0;
        }
    }
    return -1;
}

inline int _findclose(long handle) {
    if (handle > 1) {
        ModernFindHandle* h = reinterpret_cast<ModernFindHandle*>(handle);
        if (h->dir) closedir(h->dir);
        delete h;
    }
    return 0;
}
#endif
#endif

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <io.h>
#elif !defined(WINDOWS)
    // Portable fallbacks for legacy Windows types
    typedef u8   BYTE;
    typedef u16  WORD;
    typedef u32  DWORD;
    typedef s32  BOOL;
    typedef void* HINSTANCE;
    typedef void* HWND;
    typedef void* HMODULE;
    typedef void* HANDLE;
    typedef s32  HRESULT;
    typedef void* HKEY;
    typedef HKEY* PHKEY;
    typedef BYTE* LPBYTE;
    typedef DWORD* LPDWORD;
    typedef char* LPSTR;
    typedef const char* LPCSTR;
    typedef unsigned long WPARAM;
    typedef long LPARAM;
    typedef long LRESULT;
    typedef void* HMENU;
    typedef void* WNDPROC;
    inline BOOL DestroyWindow(HWND) { return 1; }
    #define TRUE 1
    #define FALSE 0
#endif

// Cross-platform calling-convention macros. Kept away from MSVC (VC6) where
// __stdcall is a built-in keyword and CC comes from the GTA2Hax headers.
#if !defined(_MSC_VER)
    #ifndef __stdcall
        #if defined(_WIN32)
            #define __stdcall __attribute__((__stdcall__))
        #else
            #define __stdcall
        #endif
    #endif

    #ifndef CC
        #define CC __stdcall
    #endif
#endif