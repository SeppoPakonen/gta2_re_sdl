#pragma once

#include "IWindowSystem.hpp"
#include "IEventSystem.hpp"
#include "IRenderer.hpp"
#include "IAudioBackend.hpp"
#include "IInputBackend.hpp"
#include "IFileSystem.hpp"
#include "IConfigBackend.hpp"
#include "ITimeSystem.hpp"
#include "IMoviePlayer.hpp"

namespace gta2::hal {

struct HalContext {
    IWindowSystem*  pWindow     = nullptr;
    IEventSystem*   pEvents     = nullptr;
    IRenderer*      pRenderer   = nullptr;
    IAudioBackend*  pAudio      = nullptr;
    IInputBackend*  pInput      = nullptr;
    IFileSystem*    pFileSystem = nullptr;
    IConfigBackend* pConfig     = nullptr;
    ITimeSystem*    pTime       = nullptr;
    IMoviePlayer*   pMovie      = nullptr;

    bool Initialize(const char* backendName);
    void Shutdown();
};

HalContext& GetHal();

// Milliseconds since the process started (steady clock); for diagnostic logs.
u64 UptimeMs();

} // namespace gta2::hal