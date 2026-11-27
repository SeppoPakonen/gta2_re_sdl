#include "memory.hpp"
#include "Function.hpp"
#include "error.hpp"
#include "crt_stubs.hpp"
#include <stdlib.h>
#include "enums.hpp"
#include <cstring>
#include <stdlib.h>

MATCH_FUNC(0x4FE4D0)
void* __stdcall Memory::malloc_4FE4D0(size_t size)
{
    void* result = crt::malloc(size);
    if (!result)
    {
        FatalError_4A38C0(Gta2Error::OutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 44, size);
    }

    return result;
}

MATCH_FUNC(0x4FE510)
void* __stdcall Memory::Aligned_malloc_4FE510(s32 requestedSize, void** pUnAlignedPtr)
{
#if defined(GTA2_MODERN_BUILD)
    // Modern build only: over-allocate and zero the block. Some legacy code reads
    // slightly past an allocation (e.g. TextureCache::ReadTextures iterates 1024
    // tile slots while the loaded tile buffer may hold fewer, and sprite/mask
    // caches are read before being fully written). On a fresh heap the over-read
    // happens to hit zeroed pages, but after the frontend/menu has run the heap
    // contains garbage, which is decoded as white/yellow texels. Giving every
    // aligned allocation generous zeroed slack makes those reads deterministic
    // (always 0) and keeps valgrind clean. The 32-bit/VC6 path is untouched.
    const size_t allocSize = static_cast<size_t>(requestedSize) + 0x40000;
    void* pAlloc = crt::malloc(allocSize);
    *pUnAlignedPtr = pAlloc;
    if (!pAlloc)
    {
        FatalError_4A38C0(Gta2Error::OutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 76, allocSize);
    }
    std::memset(pAlloc, 0, allocSize);
    return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(*pUnAlignedPtr) + 0xFFFF) & ~static_cast<uintptr_t>(0xFFFF));
#else
    // 64 KB extra so that the block can be aligned to 64 KB
    void* pAlloc = crt::malloc(requestedSize + 0x10000);

    // The caller frees this one, not the aligned pointer
    *pUnAlignedPtr = pAlloc;

    if (!pAlloc)
    {
        FatalError_4A38C0(Gta2Error::OutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 76, requestedSize + 0x10000);
    }

    return reinterpret_cast<void*>((GTA2_PTR_TO_U32(*pUnAlignedPtr) + 0xFFFF) & 0xFFFF0000);
#endif
}