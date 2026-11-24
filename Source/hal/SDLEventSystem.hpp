#pragma once

#include "IEventSystem.hpp"
#include <queue>

#if defined(GTA2_USE_SDL3)
    #include <SDL3/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

namespace gta2::hal {

class SDLEventSystem : public IEventSystem {
  private:
    bool mQuitRequested = false;
    std::queue<Event> mEventQueue;

  public:
    bool PollEvent(Event& event) override;
    void PumpEvents() override;
    bool ShouldQuit() const override { return mQuitRequested; }
    void RequestQuit() override { mQuitRequested = true; }
};

} // namespace gta2::hal