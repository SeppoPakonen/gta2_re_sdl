#pragma once

#if defined(GTA2_MODERN_BUILD)
    #include <cstdint>
    #include <cstddef>
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
#endif

// Endianness abstraction. File formats (.GMP/.STY/.SCR/.GXT) are little-endian;
// these helpers convert between little-endian on-disk order and host order.
#if defined(GTA2_MODERN_BUILD)
    #if defined(__linux__) || defined(__FreeBSD__)
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
#endif

// Portable fallbacks for legacy Windows types
#if !defined(_WIN32) && !defined(WINDOWS)
    typedef u8   BYTE;
    typedef u16  WORD;
    typedef u32  DWORD;
    typedef s32  BOOL;
    typedef void* HINSTANCE;
    typedef void* HWND;
    typedef void* HMODULE;
    typedef void* HANDLE;
    typedef s32  HRESULT;
    typedef char* LPSTR;
    typedef const char* LPCSTR;
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