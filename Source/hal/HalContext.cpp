#include "HalContext.hpp"
#include "NullMoviePlayer.hpp"
#include <cstdio>

namespace gta2::hal {

static HalContext sHalInstance;
static NullMoviePlayer sDefaultMovie;

HalContext& GetHal() {
    return sHalInstance;
}

bool HalContext::Initialize(const char* backendName) {
    printf("[HAL] Initializing backend: %s\n", backendName ? backendName : "Default");
    if (!pMovie) {
        pMovie = &sDefaultMovie;
    }
    return true;
}

void HalContext::Shutdown() {
    if (pMovie)      { pMovie->Close(); }
    if (pAudio)      { pAudio->Shutdown(); }
    if (pRenderer)   { pRenderer->Shutdown(); }
    if (pInput)      { pInput->Shutdown(); }
    if (pWindow)     { pWindow->Shutdown(); }
    if (pFileSystem) { pFileSystem->Shutdown(); }
    printf("[HAL] Shutdown complete.\n");
}

} // namespace gta2::hal