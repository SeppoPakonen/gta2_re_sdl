#include "HalContext.hpp"
#include "types.hpp"
#include "NullMoviePlayer.hpp"
#if defined(GTA2_HAVE_FFMPEG)
#include "FFmpegMoviePlayer.hpp"
#endif
#include "VfsFileSystem.hpp"
#include <chrono>
#include <cstdio>

namespace gta2::hal {

u64 UptimeMs() {
    static const std::chrono::steady_clock::time_point sStart = std::chrono::steady_clock::now();
    return static_cast<u64>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - sStart).count());
}

static HalContext sHalInstance;
static NullMoviePlayer sDefaultMovie;
#if defined(GTA2_HAVE_FFMPEG)
static FFmpegMoviePlayer sFFmpegMovie;
#endif
static VfsFileSystem sDefaultVfs;

HalContext& GetHal() {
    return sHalInstance;
}

bool HalContext::Initialize(const char* backendName) {
    GTA2_VLOG("[HAL] Initializing backend: %s\n", backendName ? backendName : "Default");
    if (!pMovie) {
#if defined(GTA2_HAVE_FFMPEG)
        pMovie = &sFFmpegMovie;
#else
        pMovie = &sDefaultMovie;
#endif
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