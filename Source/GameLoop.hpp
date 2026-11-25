#pragma once

#include "Function.hpp"

// Game loop state globals (moved from winmain.cpp so they live in gta2_core).
EXTERN_GLOBAL(u32, gMatchStartTime_6F5A28);
EXTERN_GLOBAL(u8, gFramesSinceDraw_6F58D8);
EXTERN_GLOBAL(u8, bInputsPending_6F5760);
EXTERN_GLOBAL(u8, bForceGameUpdate_6F5880);
EXTERN_GLOBAL(u8, max_frame_rate_626A08);
EXTERN_GLOBAL(u8, min_frame_rate_706C50);

// Required for GameLoop_Initialize / RunGameFrame_4DA850 — declared
// in debug.cpp (DEFINE_GLOBAL). Keeping extern declaration here matches.
EXTERN_GLOBAL(bool, bSkip_draw_67D4EA);

// Replay mode and instance handle required by InitializeGame_4DA4D0.
// bReplayMode_6F5B71 is defined in winmain.cpp; we provide the definition
// here so the modern build can link it.
EXTERN_GLOBAL(u8, bReplayMode_6F5B71);
EXTERN_GLOBAL(void*, gHInstance_708220);

// Public API of the modern game loop.
void InitializeGame_4DA4D0();
bool RunGameFrame_4DA850();
void Draw_4DA7B0();
void GameLoop_Initialize();
bool GameLoop_StepFrame();