#pragma once

#include "IWindowSystem.hpp"

#if defined(GTA2_USE_SDL3)
    #include <SDL3/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

namespace gta2::hal {

class SDLWindowSystem : public IWindowSystem {
  private:
    SDL_Window*   mWindow     = nullptr;
    SDL_GLContext mGLContext  = nullptr;
    bool          mFullscreen = false;
    s32           mWidth      = 640;
    s32           mHeight     = 480;

  public:
    bool Initialize(const char* title, s32 width, s32 height, bool fullscreen) override;
    void Shutdown() override;

    void SetTitle(const char* title) override;
    void SetWindowSize(s32 width, s32 height) override;
    void GetWindowSize(s32& width, s32& height) const override;
    void SetFullscreen(bool fullscreen) override;
    bool IsFullscreen() const override;

    void ShowCursor(bool show) override;
    void SwapBuffers() override;

    std::vector<DisplayMode> GetSupportedDisplayModes() override;
    bool SetDisplayMode(const DisplayMode& mode) override;

    void* GetNativeWindowHandle() const override { return mWindow; }
    void* GetGLContext() const { return mGLContext; }
};

} // namespace gta2::hal