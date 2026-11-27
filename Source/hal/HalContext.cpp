#include "HalContext.hpp"
#include "types.hpp"
#include "NullMoviePlayer.hpp"
#include "VfsFileSystem.hpp"
#include <cstdio>

namespace gta2::hal {

static HalContext sHalInstance;
static NullMoviePlayer sDefaultMovie;
static VfsFileSystem sDefaultVfs;

HalContext& GetHal() {
    return sHalInstance;
}

bool HalContext::Initialize(const char* backendName) {
    GTA2_VLOG("[HAL] Initializing backend: %s\n", backendName ? backendName : "Default");
    if (!pMovie) {
        pMovie = &sDefaultMovie;
    }
    if (!pFileSystem) {
        sDefaultVfs.Initialize(".", ".");
        pFileSystem = &sDefaultVfs;
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
    GTA2_VLOG("[HAL] Shutdown complete.\n");
}

} // namespace gta2::hal