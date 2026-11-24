#pragma once

#include "Function.hpp"
#if !defined(GTA2_MODERN_BUILD)
#include "3rdParty/GTA2Hax/DmaVideo/DmaVideo.hpp"
#include <Windows.h>
#else
struct SVideo {
    u32 field_0;
    u32 field_4_flags;
    u32 field_8_found_width;
    u32 field_C_found_height;
    u32 field_10_found_rgb_bit_count;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
    u32 field_20;
    void* field_24;
    void* field_28;
    void* field_2C;
    void* field_30;
    u32 field_34;
    u32 field_38;
    void* field_3C;
    u32 field_40_full_screen;
    u32 field_44;
    u32 field_48_rect_right;
    u32 field_4C_rect_bottom;
    void* field_50_surface_pixels_ptr;
    u32 field_54_surface_pixels_pitch;
};

struct SVidVersion {
    u32 mVersion;
    char mVersionString[255];
};

typedef SVidVersion* (*T_Vid_GetVersion)();
typedef SVideo* (*T_Vid_Init_SYS)(void*, u16);
typedef s32 (*T_Vid_CheckMode)(SVideo*, s32, s32, s32);
typedef s32 (*T_Vid_FindMode)(SVideo*, s32, s32, s32);
typedef s32 (*T_Vid_FindFirstMode)(SVideo*);
typedef s32 (*T_Vid_FindNextMode)(SVideo*);
typedef s32 (*T_Vid_FindDevice)(SVideo*);
typedef s32 (*T_Vid_SetDevice)(SVideo*, s32);
typedef s32 (*T_Vid_CloseScreen)(SVideo*);
typedef s32 (*T_Vid_SetMode)(SVideo*, void*, s32);
typedef void (*T_Vid_FlipBuffers)(SVideo*);
typedef s32 (*T_Vid_ReleaseSurface)(SVideo*, void*);
typedef s32 (*T_Vid_GrabSurface)(SVideo*, void*);
typedef s32 (*T_Vid_ShutDown_SYS)(SVideo*);
typedef s32 (*T_Vid_EnableWrites)(SVideo*);
typedef s32 (*T_Vid_DisableWrites)(SVideo*);
typedef s32 (*T_Vid_GetSurface)(SVideo*);
typedef s32 (*T_Vid_FreeSurface)(SVideo*);
typedef s32 (*T_Vid_ClearScreen)(SVideo*, u8, u8, u8, s32, s32, s32, s32);
typedef void* (*T_Vid_WindowProc)(void*, u32, u32, s32);
typedef s32 (*T_Vid_InitDLL)(void*, void*);
typedef s32 (*T_Vid_SetGamma)(SVideo*, f32, f32, f32);
#endif

EXTERN_GLOBAL(SVideo*, gVidSys_7071D0);

EXTERN_GLOBAL(T_Vid_GetVersion, pVid_GetVersion);
EXTERN_GLOBAL(T_Vid_Init_SYS, pVid_Init_SYS);
EXTERN_GLOBAL(T_Vid_CheckMode, pVid_CheckMode);
EXTERN_GLOBAL(T_Vid_FindMode, pVid_FindMode);
EXTERN_GLOBAL(T_Vid_FindFirstMode, pVid_FindFirstMode);
EXTERN_GLOBAL(T_Vid_FindNextMode, pVid_FindNextMode);
EXTERN_GLOBAL(T_Vid_FindDevice, pVid_FindDevice);
EXTERN_GLOBAL(T_Vid_SetDevice, pVid_SetDevice);
EXTERN_GLOBAL(T_Vid_CloseScreen, pVid_CloseScreen);
EXTERN_GLOBAL(T_Vid_SetMode, pVid_SetMode);
EXTERN_GLOBAL(T_Vid_FlipBuffers, pVid_FlipBuffers);
EXTERN_GLOBAL(T_Vid_ReleaseSurface, pVid_ReleaseSurface);
EXTERN_GLOBAL(T_Vid_GrabSurface, pVid_GrabSurface);
EXTERN_GLOBAL(T_Vid_ShutDown_SYS, pVid_ShutDown_SYS);
EXTERN_GLOBAL(T_Vid_EnableWrites, pVid_EnableWrites);
EXTERN_GLOBAL(T_Vid_DisableWrites, pVid_DisableWrites);
EXTERN_GLOBAL(T_Vid_GetSurface, pVid_GetSurface);
EXTERN_GLOBAL(T_Vid_FreeSurface, pVid_FreeSurface);
EXTERN_GLOBAL(T_Vid_ClearScreen, pVid_ClearScreen);
EXTERN_GLOBAL(T_Vid_WindowProc, pVid_WindowProc);
EXTERN_GLOBAL(T_Vid_InitDLL, pVid_InitDLL);
EXTERN_GLOBAL(T_Vid_SetGamma, pVid_SetGamma);

EXPORT void __stdcall DMA_Video_Stub_5EB810();
EXPORT void __stdcall DMA_Video_Unload_5EB8C0(SVideo* pVidSys);
EXPORT s32 __stdcall DMA_Video_LoadDll_5EB970(const char_type* lpLibFileName);