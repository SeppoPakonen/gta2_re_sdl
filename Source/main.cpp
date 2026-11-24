#include "EngineRunner.hpp"

#if defined(__EMSCRIPTEN__)
    #include <emscripten.h>

static void EmscriptenLoop(void* arg) {
    gta2::EngineRunner* runner = static_cast<gta2::EngineRunner*>(arg);
    if (!runner->StepFrame()) {
        emscripten_cancel_main_loop();
        runner->Shutdown();
    }
}
#endif

int main(int argc, char** argv) {
    gta2::EngineRunner runner;
    if (!runner.Initialize(argc, argv)) {
        return 1;
    }

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(EmscriptenLoop, &runner, 0, 1);
#else
    while (runner.StepFrame()) {
        // Frame rate limiter / VSync handled by HAL
    }
    runner.Shutdown();
#endif

    return 0;
}