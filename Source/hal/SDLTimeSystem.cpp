#include "SDLTimeSystem.hpp"
#include <chrono>
#include <thread>

#if defined(GTA2_USE_SDL3)
    #include <SDL3/SDL.h>
#elif defined(GTA2_USE_SDL2)
    #include <SDL2/SDL.h>
#endif

namespace gta2::hal {

u64 SDLTimeSystem::GetTicksMs() {
#if defined(GTA2_USE_SDL2) || defined(GTA2_USE_SDL3)
    return SDL_GetTicks();
#else
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
#endif
}

u64 SDLTimeSystem::GetTicksMicroseconds() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

void SDLTimeSystem::SleepMs(u32 milliseconds) {
#if defined(GTA2_USE_SDL2) || defined(GTA2_USE_SDL3)
    SDL_Delay(milliseconds);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

} // namespace gta2::hal