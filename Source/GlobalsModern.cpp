#include "Function.hpp"
#include "NetUi.hpp"

// Engine state globals required by the modern build.
// These are normally defined in winmain.cpp (excluded from gta2_core)
// but must be available for the core library to link.
DEFINE_GLOBAL(s32, window_width_706630, 640);
DEFINE_GLOBAL(s32, window_height_706B50, 480);
DEFINE_GLOBAL(char_type, gLighting_626A09, 0);
DEFINE_GLOBAL(u32, gLightingDrawFlag_7068F4, 0);
DEFINE_GLOBAL(u32, gBufferMode_706B34, 0);
DEFINE_GLOBAL(bool, bStartNetworkGame_7081F0, false);
DEFINE_GLOBAL(s32, gGTA2VersionMajor_708280, 10);
DEFINE_GLOBAL(s32, gGTA2VersionMinor_708284, 5);
DEFINE_GLOBAL(u8, bDoFrontEnd_626B68, 1);
DEFINE_GLOBAL(NetworkGameSettings, gNetworkGameSettings_707098, {});

EXPORT void Start_GTA2Manager_5E4DE0() {}
EXPORT void UpdateGameScreenSize_5D8E00() {}
EXPORT void ResetFrameTimer_4DA830() {}
EXPORT void ErrorMsgBox_5E4EC0(const char* msg) { if (msg) printf("[ERROR] %s\n", msg); }
EXPORT void Shutdown_4DA740() {}
EXPORT void GBH_Graphis_DMA_Video_Free_5D9830() {}