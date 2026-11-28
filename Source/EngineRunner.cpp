#include "EngineRunner.hpp"
#include "hal/HalContext.hpp"
#include "hal/VfsFileSystem.hpp"
#include "hal/SDLWindowSystem.hpp"
#include "hal/SDLEventSystem.hpp"
#include "hal/SDLTimeSystem.hpp"
#include "hal/OpenGLRenderer.hpp"
#include "hal/SDLAudioBackend.hpp"
#include "hal/SDLInputBackend.hpp"
#include "GameLoop.hpp"
#include "gbh_graphics.hpp"
#include "dma_video.hpp"
#include <cstdlib>
#include <string>

void GBH_GraphicsInit_Modern();
EXTERN_GLOBAL(u8, bReplayMode_6F5B71);

namespace gta2 {

// Set by the --skipintro command line flag; read by the frontend to jump
// straight to the start menu instead of playing the intro movie.
bool gSkipIntro_Requested = false;

bool EngineRunner::Initialize(int argc, char** argv) {
    auto& ctx = hal::GetHal();
    if (ctx.pFileSystem) {
        ctx.pFileSystem->Shutdown();
    }

    static hal::VfsFileSystem vfs;
    vfs.Initialize(".", nullptr);
    vfs.AddSearchRoot("assets");
    if (const char* pDataDir = std::getenv("GTA2_DATA_DIR")) {
        if (*pDataDir) {
            vfs.AddSearchRoot(pDataDir);
        }
    }
    if (const char* home = std::getenv("HOME")) {
        std::string defaultWine = std::string(home) + "/.wine/drive_c/Program Files (x86)/Rockstar Games/GTA2";
        vfs.AddSearchRoot(defaultWine.c_str());
    }
    ctx.pFileSystem = &vfs;

    static hal::SDLTimeSystem timeSys;
    static hal::SDLWindowSystem winSys;
    static hal::SDLEventSystem evtSys;
    static hal::OpenGLRenderer glRenderer;
    static hal::SDLAudioBackend audioBackend;
    static hal::SDLInputBackend inputBackend;

    ctx.pTime = &timeSys;
    ctx.pWindow = &winSys;
    ctx.pEvents = &evtSys;
    ctx.pRenderer = &glRenderer;
    ctx.pAudio = &audioBackend;
    ctx.pInput = &inputBackend;

    // Check command line arguments for --demo or live mode
    bReplayMode_6F5B71 = 0; // Default to normal game / frontend menu
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--nodemo" || arg == "--game") {
            bReplayMode_6F5B71 = 0;
        } else if (arg == "--demo") {
            bReplayMode_6F5B71 = 1;
        } else if (arg == "--skipintro" || arg == "--skip-intro" || arg == "--no-intro") {
            gSkipIntro_Requested = true;
        }
    }

    // Initialize window and graphics
    if (winSys.Initialize("GTA2 Modern SDL", 640, 480, false)) {
        glRenderer.SetProcLoader(reinterpret_cast<void*(*)(const char*)>(SDL_GL_GetProcAddress));
        glRenderer.Initialize(&winSys);
    }
    audioBackend.Initialize(22050, 2, 1024);
    inputBackend.Initialize();

    // Finalize the HAL context (selects the movie player, default VFS, etc.).
    // The individual backends were assigned above; this fills in the rest.
    ctx.Initialize("SDL");

    DMA_Video_LoadDll_5EB970(nullptr);
    GBH_GraphicsInit_Modern();

    // Start the game simulation via the modern game loop.
    GameLoop_Initialize();

    mIsRunning = true;
    mInFrontend = false;
    return true;
}

bool EngineRunner::StepFrame() {
    return GameLoop_StepFrame();
}

void EngineRunner::Shutdown() {
    mIsRunning = false;
    hal::GetHal().Shutdown();
}

} // namespace gta2