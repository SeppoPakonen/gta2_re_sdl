#pragma once
#include "HalTypes.hpp"
#include <vector>

namespace gta2::hal {

class IWindowSystem {
public:
    virtual ~IWindowSystem() = default;

    virtual bool Initialize(const char* title, s32 width, s32 height, bool fullscreen) = 0;
    virtual void Shutdown() = 0;

    virtual void SetTitle(const char* title) = 0;
    virtual void SetWindowSize(s32 width, s32 height) = 0;
    virtual void GetWindowSize(s32& width, s32& height) const = 0;
    virtual void SetFullscreen(bool fullscreen) = 0;
    virtual bool IsFullscreen() const = 0;

    virtual void ShowCursor(bool show) = 0;
    virtual void SwapBuffers() = 0;

    virtual std::vector<DisplayMode> GetSupportedDisplayModes() = 0;
    virtual bool SetDisplayMode(const DisplayMode& mode) = 0;

    virtual void* GetNativeWindowHandle() const = 0; // SDL_Window* or native HWND/ANativeWindow
};

} // namespace gta2::hal