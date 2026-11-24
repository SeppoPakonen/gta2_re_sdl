#include "EngineRunner.hpp"
#include "hal/HalContext.hpp"

namespace gta2 {

bool EngineRunner::Initialize(int argc, char** argv) {
    (void)argc;
    (void)argv;
    mIsRunning = true;
    mInFrontend = true;
    return true;
}

bool EngineRunner::StepFrame() {
    auto& ctx = hal::GetHal();

    // 1. Process OS events
    if (ctx.pEvents) {
        ctx.pEvents->PumpEvents();
        if (ctx.pEvents->ShouldQuit()) {
            return false;
        }
    }

    // 2. Update input & audio
    if (ctx.pInput) {
        ctx.pInput->Update();
    }
    if (ctx.pAudio) {
        ctx.pAudio->Update();
    }

    // 3. Render and simulate frame
    if (mInFrontend) {
        // Run Frontend menu frame
    } else {
        // RunGameFrame_4DA850();
    }

    // 4. Swap display buffers
    if (ctx.pWindow) {
        ctx.pWindow->SwapBuffers();
    }

    return mIsRunning;
}

void EngineRunner::Shutdown() {
    mIsRunning = false;
}

} // namespace gta2