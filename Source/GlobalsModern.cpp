#include "Function.hpp"
#include "NetUi.hpp"

// Engine state globals required by the modern build.
// These are normally defined in winmain.cpp (excluded from gta2_core)
// but must be available for the core library to link.
DEFINE_GLOBAL(s32, window_width_706630, 0x706630);
DEFINE_GLOBAL(s32, window_height_706B50, 0x706B50);
DEFINE_GLOBAL(char_type, gLighting_626A09, 0x626A09);
DEFINE_GLOBAL(u32, gLightingDrawFlag_7068F4, 0x7068F4);
DEFINE_GLOBAL(u32, gBufferMode_706B34, 0x706B34);
DEFINE_GLOBAL(s32, bStartNetworkGame_7081F0, 0x7081F0);
DEFINE_GLOBAL(s32, gGTA2VersionMajor_708280, 0x708280);
DEFINE_GLOBAL(s32, gGTA2VersionMinor_708284, 0x708284);
DEFINE_GLOBAL_INIT(u8, bDoFrontEnd_626B68, true, 0x626B68);
DEFINE_GLOBAL(NetworkGameSettings, gNetworkGameSettings_707098, 0x707098);
DEFINE_GLOBAL(u8, bReplayMode_6F5B71, 0x6F5B71);
DEFINE_GLOBAL(void*, gHInstance_708220, 0x708220);

// The original "Options" entry launches the external "GTA2 Manager.EXE" to change
// display/audio settings. That separate Windows app is not part of this cross-platform
// port, so keep the action a safe no-op but log it instead of failing silently.
EXPORT void Start_GTA2Manager_5E4DE0() { printf("[OPTIONS] GTA2 Manager is not available in the modern build\n"); }
EXPORT void UpdateGameScreenSize_5D8E00() {}
EXPORT void ResetFrameTimer_4DA830() {}
EXPORT void ErrorMsgBox_5E4EC0(const char* msg) { if (msg) printf("[ERROR] %s\n", msg); }
EXPORT void Shutdown_4DA740() {}
EXPORT void GBH_Graphis_DMA_Video_Free_5D9830() {}