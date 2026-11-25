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
#include "Draw.hpp"
#include "Char_Pool.hpp"
#include "Car.hpp"
#include "Ped.hpp"
#include "Player.hpp"
#include "rng.hpp"
#include "Frontend.hpp"
#include <cstdlib>
#include <cstdio>

// Game loop state global definitions.
DEFINE_GLOBAL(u32, gMatchStartTime_6F5A28, 0x6F5A28);
DEFINE_GLOBAL(u8, gFramesSinceDraw_6F58D8, 0x6F58D8);
DEFINE_GLOBAL(u8, bInputsPending_6F5760, 0x6F5760);
DEFINE_GLOBAL(u8, bForceGameUpdate_6F5880, 0x6F5880);
DEFINE_GLOBAL_INIT(u8, max_frame_rate_626A08, 1, 0x626A08);
DEFINE_GLOBAL(u8, min_frame_rate_706C50, 0x706C50);
EXTERN_GLOBAL(bool, bSkip_draw_67D4EA);
EXTERN_GLOBAL(u8, bDoFrontEnd_626B68);
EXTERN_GLOBAL(u8, bReplayMode_6F5B71);
EXTERN_GLOBAL(void*, gHInstance_708220);

static bool bInitialized = false;

void InitializeGame_4DA4D0() {
    min_frame_rate_706C50 = 1;
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
                const u32 inputs = gInputRecorder_67F8B0.get_input_bits_4CEAC0();
                if (gGame_0x40_67E008 && gGame_0x40_67E008->field_38_local_player) {
                    gGame_0x40_67E008->field_38_local_player->SetInputs_565740(inputs);
                }
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
            const u32 inputs = gInputRecorder_67F8B0.get_input_bits_4CEAC0();
            if (gGame_0x40_67E008 && gGame_0x40_67E008->field_38_local_player) {
                gGame_0x40_67E008->field_38_local_player->SetInputs_565740(inputs);
            }
            bInputsPending_6F5760 = false;
        }
    }

    if (!drew_frame && !updated_logic && gta2::hal::GetHal().pTime) {
        gta2::hal::GetHal().pTime->SleepMs(1);
    }

    return exit_requested;
}

static int sTotalDrawnFrames = 0;

void Draw_4DA7B0() {
    gGame_0x40_67E008->Draw_4B92D0();
    DiagnosticUpdateTextEvents();

    sTotalDrawnFrames++;

    const char* pScreenshotEnv = std::getenv("GTA2_SCREENSHOT_PATH");
    const char* pMaxFramesEnv = std::getenv("GTA2_DEMO_FRAMES");
    const int maxFrames = pMaxFramesEnv ? std::atoi(pMaxFramesEnv) : 0;
    if (pScreenshotEnv && gta2::hal::GetHal().pRenderer &&
        (sTotalDrawnFrames == 1 || sTotalDrawnFrames == 5 || sTotalDrawnFrames == (maxFrames > 0 ? (maxFrames / 2) : 50))) {
        bool ok = gta2::hal::GetHal().pRenderer->CaptureScreenshot(pScreenshotEnv);
        std::printf("[Screenshot] Saved frame %d to %s: %s\n", sTotalDrawnFrames, pScreenshotEnv, ok ? "SUCCESS" : "FAILED");
    }

    pVid_FlipBuffers(gVidSys_7071D0);
    if (bSkip_lid_67D546 || bSkip_top_67D574 || bSkip_bottom_67D4E7
        || bSkip_left_67D6BF || bSkip_right_67D4E4) {
        pVid_ClearScreen(gVidSys_7071D0, 0, 0, 0, 0, 0,
                         gVidSys_7071D0->field_48_rect_right,
                         gVidSys_7071D0->field_4C_rect_bottom);
    }
}

void GameLoop_Initialize() {
    gRoot_sound_66B038.Init_40EF80();
    KeyboardLayout::create_4D5F50();
    PlyDat::Create_56C2C0();

    if (bReplayMode_6F5B71 != 0) {
        bDoFrontEnd_626B68 = 0;
        InitializeGame_4DA4D0();
    } else {
        bDoFrontEnd_626B68 = 1;
        Frontend::create_4ACFA0();
        if (gFrontend_67DC84) {
            gFrontend_67DC84->ChangeMenuPage_4B3170(MENUPAGE_START_MENU);
        }
    }
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

    if (bDoFrontEnd_626B68 && gFrontend_67DC84) {
        s32 state = gFrontend_67DC84->Run_4AEDB0();
        sTotalDrawnFrames++;

        const char* pScreenshotEnv = std::getenv("GTA2_SCREENSHOT_PATH");
        if (pScreenshotEnv && gta2::hal::GetHal().pRenderer && (sTotalDrawnFrames == 5 || sTotalDrawnFrames == 20)) {
            gta2::hal::GetHal().pRenderer->CaptureScreenshot(pScreenshotEnv);
            std::printf("[Screenshot] Saved frontend frame %d to %s\n", sTotalDrawnFrames, pScreenshotEnv);
        }

        pVid_FlipBuffers(gVidSys_7071D0);

        if (state == Quit_1) {
            Frontend::destroy_4AD070();
            bDoFrontEnd_626B68 = 0;
            return false;
        } else if (state == Start_Game_3) {
            Frontend::destroy_4AD070();
            bDoFrontEnd_626B68 = 0;
            bReplayMode_6F5B71 = 0;
            InitializeGame_4DA4D0();
            return true;
        } else if (state == Start_Replay_4) {
            Frontend::destroy_4AD070();
            bDoFrontEnd_626B68 = 0;
            bReplayMode_6F5B71 = 1;
            InitializeGame_4DA4D0();
            return true;
        }
        return true;
    }

    // Run a game frame (pVid_FlipBuffers inside Draw_4DA7B0 swaps buffers when a frame is rendered).
    if (RunGameFrame_4DA850()) {
        return false;
    }

    static int sFrameCount = 0;
    sFrameCount++;

    // Periodic simulation diagnostics (player pos, health, RNG cycle, entity counts)
    if (sFrameCount % 60 == 0) {
        if (gGame_0x40_67E008 && gGame_0x40_67E008->field_38_local_player) {
            Player* pPlayer = gGame_0x40_67E008->field_38_local_player;
            Ped* pPed = pPlayer->GetPlayerPed_41D020();
            if (pPed) {
                float px = 0.0f, py = 0.0f, pz = 0.0f;
                if (pPed->field_168_game_object && pPed->field_168_game_object->field_80_sprite_ptr) {
                    px = pPed->field_168_game_object->field_80_sprite_ptr->field_14_xy.x.ToFloat();
                    py = pPed->field_168_game_object->field_80_sprite_ptr->field_14_xy.y.ToFloat();
                    pz = pPed->field_168_game_object->field_80_sprite_ptr->field_1C_zpos.ToFloat();
                } else {
                    px = pPed->field_1AC_cam.x.ToFloat();
                    py = pPed->field_1AC_cam.y.ToFloat();
                    pz = pPed->field_1AC_cam.z.ToFloat();
                }
                const s32 health = pPed->get_health_433B70();
                const u32 rng = gpRng_67AB34 ? gpRng_67AB34->get_cur_rng_41CFE0() : 0;
                std::printf("[DIAG-PLAYER] Frame %d: pos=(%.2f, %.2f, %.2f) health=%d rng=%u\n",
                            sFrameCount, px, py, pz, health, rng);
            }
        }
        if (gPedManager_6787BC || gCarManager_6C_677930) {
            int pedsOnScreen = gPedManager_6787BC ? gPedManager_6787BC->field_6_num_peds_on_screen : 0;
            int pedsUpdated = gPedManager_6787BC ? gPedManager_6787BC->field_3_num_peds_updated : 0;
            int pedsDummy = gPedManager_6787BC ? gPedManager_6787BC->field_2_num_dummy_chars : 0;
            int carsVisible = gCarManager_6C_677930 ? gCarManager_6C_677930->field_55_visible_cars_count : 0;
            int carsRecycled = gCarManager_6C_677930 ? gCarManager_6C_677930->field_28_recycled_cars : 0;
            std::printf("[DIAG-ENTITIES] Frame %d: peds(on_screen=%d, updated=%d, dummy=%d), cars(visible=%d, recycled=%d)\n",
                        sFrameCount, pedsOnScreen, pedsUpdated, pedsDummy, carsVisible, carsRecycled);
        }
        std::fflush(stdout);
    }

    const char* pMaxFramesEnv = std::getenv("GTA2_DEMO_FRAMES");
    const int maxFrames = pMaxFramesEnv ? std::atoi(pMaxFramesEnv) : 0;

    if (maxFrames > 0 && sTotalDrawnFrames >= maxFrames) {
        return false;
    }

    return true;
}
