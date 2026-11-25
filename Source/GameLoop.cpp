#include "GameLoop.hpp"
#include "hal/HalContext.hpp"
#include "hal/VfsFileSystem.hpp"
#include "hal/SDLWindowSystem.hpp"
#include "hal/SDLEventSystem.hpp"
#include "hal/OpenGLRenderer.hpp"
#include "hal/SDLAudioBackend.hpp"
#include "hal/SDLInputBackend.hpp"
#include "hal/SDLTimeSystem.hpp"
#include "gbh_graphics.hpp"
#include "dma_video.hpp"
#include "InputRecorder_67F8B0.hpp"
#include "Game.hpp"
#include "Function.hpp"
#include "debug.hpp"
#include "root_sound.hpp"
#include "registry.hpp"
#include "KeyboardLayout.hpp"
#include "PlyDat.hpp"
#include <cstdlib>

// Game loop state global definitions.
DEFINE_GLOBAL(u32, gMatchStartTime_6F5A28, 0x6F5A28);
DEFINE_GLOBAL(u8, gFramesSinceDraw_6F58D8, 0x6F58D8);
DEFINE_GLOBAL(u8, bInputsPending_6F5760, 0x6F5760);
DEFINE_GLOBAL(u8, bForceGameUpdate_6F5880, 0x6F5880);
DEFINE_GLOBAL_INIT(u8, max_frame_rate_626A08, 1, 0x626A08);
DEFINE_GLOBAL(u8, min_frame_rate_706C50, 0x706C50);
EXTERN_GLOBAL(bool, bSkip_draw_67D4EA);
DEFINE_GLOBAL(u8, bReplayMode_6F5B71, 0);
DEFINE_GLOBAL(void*, gHInstance_708220, nullptr);

static bool bInitialized = false;

void InitializeGame_4DA4D0() {
    KeyboardLayout::create_4D5F50();
    PlyDat::Create_56C2C0();

    if (bReplayMode_6F5B71) {
        gInputRecorder_67F8B0.input_init_replay_4CE740(static_cast<HINSTANCE>(gHInstance_708220));
    } else {
        gInputRecorder_67F8B0.input_init_live_4CE880(static_cast<HINSTANCE>(gHInstance_708220));
    }

    gRoot_sound_66B038.Set3DSound_40F160(gRegistry_6FF968.Sound_Setting_Exists_586A70("do_3d_sound"));
    gRegistry_6FF968.Clear_Or_Delete_Sound_Setting_586BF0("do_3d_sound", gRoot_sound_66B038.Get3DSound_40F180());

    gGame_0x40_67E008 = new Game(1, 0);
    gGame_0x40_67E008->LoadGameFiles_4B8C40();
    gGame_0x40_67E008->BootGame_4B8EB0();

    gMatchStartTime_6F5A28 = static_cast<u32>(gta2::hal::GetHal().pTime->GetTicksMs());
    gFramesSinceDraw_6F58D8 = 0;
    bForceGameUpdate_6F5880 = false;
    bInputsPending_6F5760 = true;
}

bool RunGameFrame_4DA850() {
    bool exit_requested = false;
    bool drew_frame = false;
    bool updated_logic = false;
    const u32 Time = static_cast<u32>(gta2::hal::GetHal().pTime->GetTicksMs());

    if (false) {
        // Network path: enabled later.
        if (Time >= gMatchStartTime_6F5A28) {
            exit_requested = gGame_0x40_67E008->ExecuteGame_4B9640();
            gMatchStartTime_6F5A28 = Time
                + static_cast<u32>(gGame_0x40_67E008->GetFrameDurationMs_4B8BB0());
        }
    } else {
        if (!max_frame_rate_626A08 && !gFramesSinceDraw_6F58D8 || Time >= gMatchStartTime_6F5A28) {
            updated_logic = true;
        }
        if (gFramesSinceDraw_6F58D8 >= 8u || !min_frame_rate_706C50 && gFramesSinceDraw_6F58D8) {
            updated_logic = false;
        }

        if (bForceGameUpdate_6F5880 || bSkip_draw_67D4EA) {
            updated_logic = true;
        }

        if (updated_logic) {
            if (!min_frame_rate_706C50) {
                gMatchStartTime_6F5A28 = Time;
            }
            if (bInputsPending_6F5760) {
                bInputsPending_6F5760 = false;
            }
            exit_requested = gGame_0x40_67E008->ExecuteGame_4B9640();
            const s32 frame_ms = gGame_0x40_67E008->GetFrameDurationMs_4B8BB0();
            bForceGameUpdate_6F5880 = false;
            gMatchStartTime_6F5A28 += static_cast<u32>(frame_ms);
            ++gFramesSinceDraw_6F58D8;
            bInputsPending_6F5760 = true;
        } else if (gFramesSinceDraw_6F58D8) {
            Draw_4DA7B0();
            gFramesSinceDraw_6F58D8 = 0;
            drew_frame = true;
        } else if (bInputsPending_6F5760) {
            bInputsPending_6F5760 = false;
        }
    }
    return exit_requested;
}

void Draw_4DA7B0() {
    gGame_0x40_67E008->Draw_4B92D0();
    pVid_FlipBuffers(gVidSys_7071D0);
    if (bSkip_lid_67D546 || bSkip_top_67D574 || bSkip_bottom_67D4E7
        || bSkip_left_67D6BF || bSkip_right_67D4E4) {
        pVid_ClearScreen(gVidSys_7071D0, 0, 0, 0, 0, 0,
                         gVidSys_7071D0->field_48_rect_right,
                         gVidSys_7071D0->field_4C_rect_bottom);
    }
}

void GameLoop_Initialize() {
    // Initialize HAL subsystems already done by EngineRunner.
    // Just start the game simulation (equivalent of legacy InitializeGame_4DA4D0).
    InitializeGame_4DA4D0();
    bInitialized = true;
}

bool GameLoop_StepFrame() {
    // Pump OS events.
    gta2::hal::GetHal().pEvents->PumpEvents();
    if (gta2::hal::GetHal().pEvents->ShouldQuit()) {
        return false;
    }

    // Update input and audio.
    if (gta2::hal::GetHal().pInput) {
        gta2::hal::GetHal().pInput->Update();
    }
    if (gta2::hal::GetHal().pAudio) {
        gta2::hal::GetHal().pAudio->Update();
    }

    // Run a game frame.
    if (RunGameFrame_4DA850()) {
        return false;
    }

    // Swap display buffers via HAL.
    if (gta2::hal::GetHal().pWindow) {
        gta2::hal::GetHal().pWindow->SwapBuffers();
    }

    static int sFrameCount = 0;
    sFrameCount++;

    const char* pMaxFramesEnv = std::getenv("GTA2_DEMO_FRAMES");
    const int maxFrames = pMaxFramesEnv ? std::atoi(pMaxFramesEnv) : 0;

    const char* pScreenshotEnv = std::getenv("GTA2_SCREENSHOT_PATH");
    if (pScreenshotEnv && gta2::hal::GetHal().pRenderer &&
        (sFrameCount == (maxFrames > 0 ? (maxFrames / 2) : 100) || sFrameCount == maxFrames)) {
        gta2::hal::GetHal().pRenderer->CaptureScreenshot(pScreenshotEnv);
    }

    if (maxFrames > 0 && sFrameCount >= maxFrames) {
        return false;
    }

    return true;
}
