#include "SDLWindowSystem.hpp"

namespace gta2::hal {

#if defined(GTA2_USE_SDL3)

bool SDLWindowSystem::Initialize(const char* title, s32 width, s32 height, bool fullscreen) {
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        return false;
    }
    mWidth = width;
    mHeight = height;
    mFullscreen = fullscreen;

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags = static_cast<SDL_WindowFlags>(flags | SDL_WINDOW_FULLSCREEN);
    }
    SDL_SetHint("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", "0");
    mWindow = SDL_CreateWindow(title ? title : "GTA2", width, height, flags);
    if (!mWindow) {
        return false;
    }
    mGLContext = SDL_GL_CreateContext(mWindow);
    return mGLContext != nullptr;
}

void SDLWindowSystem::Shutdown() {
    if (mGLContext) {
        SDL_GL_DeleteContext(mGLContext);
        mGLContext = nullptr;
    }
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SDLWindowSystem::SetTitle(const char* title) {
    if (mWindow) {
        SDL_SetWindowTitle(mWindow, title ? title : "");
    }
}

void SDLWindowSystem::SetWindowSize(s32 width, s32 height) {
    mWidth = width;
    mHeight = height;
    if (mWindow) {
        SDL_SetWindowSize(mWindow, width, height);
    }
}

void SDLWindowSystem::GetWindowSize(s32& width, s32& height) const {
    if (mWindow) {
        int w = 0, h = 0;
        SDL_GetWindowSize(mWindow, &w, &h);
        width = w;
        height = h;
    } else {
        width = mWidth;
        height = mHeight;
    }
}

void SDLWindowSystem::SetFullscreen(bool fullscreen) {
    mFullscreen = fullscreen;
    if (mWindow) {
        SDL_SetWindowFullscreen(mWindow, fullscreen);
    }
}

bool SDLWindowSystem::IsFullscreen() const {
    return mFullscreen;
}

void SDLWindowSystem::ShowCursor(bool show) {
    SDL_ShowCursor(show);
}

void SDLWindowSystem::SwapBuffers() {
    if (mWindow) {
        SDL_GL_SwapWindow(mWindow);
    }
}

std::vector<DisplayMode> SDLWindowSystem::GetSupportedDisplayModes() {
    std::vector<DisplayMode> modes;
    if (!mWindow) {
        return modes;
    }
    int display = SDL_GetWindowDisplayIndex(mWindow);
    int count = 0;
    SDL_DisplayMode** pModes = SDL_GetFullscreenDisplayModes(display, &count);
    if (pModes) {
        for (int i = 0; i < count; ++i) {
            const SDL_DisplayMode* m = pModes[i];
            if (!m) {
                continue;
            }
            DisplayMode dm;
            dm.width = m->w;
            dm.height = m->h;
            dm.refreshRate = static_cast<s32>(m->refresh_rate);
            dm.bitsPerPixel = SDL_BITSPERPIXEL(m->format);
            modes.push_back(dm);
        }
        SDL_free(pModes);
    }
    return modes;
}

bool SDLWindowSystem::SetDisplayMode(const DisplayMode& mode) {
    if (!mWindow) {
        return false;
    }
    SDL_DisplayMode target;
    target.format = 0;
    target.w = mode.width;
    target.h = mode.height;
    target.refresh_rate = mode.refreshRate;
    target.driverdata = nullptr;
    SDL_SetWindowDisplayMode(mWindow, &target);
    SDL_SetWindowSize(mWindow, mode.width, mode.height);
    mWidth = mode.width;
    mHeight = mode.height;
    return true;
}

#else // SDL2

bool SDLWindowSystem::Initialize(const char* title, s32 width, s32 height, bool fullscreen) {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return false;
    }
    mWidth = width;
    mHeight = height;
    mFullscreen = fullscreen;

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL_SetHint("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", "0");
    mWindow = SDL_CreateWindow(title ? title : "GTA2",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width,
                               height,
                               flags);
    if (!mWindow) {
        return false;
    }
    mGLContext = SDL_GL_CreateContext(mWindow);
    return mGLContext != nullptr;
}

void SDLWindowSystem::Shutdown() {
    if (mGLContext) {
        SDL_GL_DeleteContext(mGLContext);
        mGLContext = nullptr;
    }
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SDLWindowSystem::SetTitle(const char* title) {
    if (mWindow) {
        SDL_SetWindowTitle(mWindow, title ? title : "");
    }
}

void SDLWindowSystem::SetWindowSize(s32 width, s32 height) {
    mWidth = width;
    mHeight = height;
    if (mWindow) {
        SDL_SetWindowSize(mWindow, width, height);
    }
}

void SDLWindowSystem::GetWindowSize(s32& width, s32& height) const {
    if (mWindow) {
        int w = 0, h = 0;
        SDL_GetWindowSize(mWindow, &w, &h);
        width = w;
        height = h;
    } else {
        width = mWidth;
        height = mHeight;
    }
}

void SDLWindowSystem::SetFullscreen(bool fullscreen) {
    mFullscreen = fullscreen;
    if (mWindow) {
        SDL_SetWindowFullscreen(mWindow, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}

bool SDLWindowSystem::IsFullscreen() const {
    return mFullscreen;
}

void SDLWindowSystem::ShowCursor(bool show) {
    SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}

void SDLWindowSystem::SwapBuffers() {
    if (mWindow) {
        SDL_GL_SwapWindow(mWindow);
    }
}

std::vector<DisplayMode> SDLWindowSystem::GetSupportedDisplayModes() {
    std::vector<DisplayMode> modes;
    int display = mWindow ? SDL_GetWindowDisplayIndex(mWindow) : 0;
    int num = SDL_GetNumDisplayModes(display);
    for (int i = 0; i < num; ++i) {
        SDL_DisplayMode m;
        if (SDL_GetDisplayMode(display, i, &m) != 0) {
            continue;
        }
        DisplayMode dm;
        dm.width = m.w;
        dm.height = m.h;
        dm.refreshRate = m.refresh_rate;
        dm.bitsPerPixel = SDL_BITSPERPIXEL(m.format);
        modes.push_back(dm);
    }
    return modes;
}

bool SDLWindowSystem::SetDisplayMode(const DisplayMode& mode) {
    if (!mWindow) {
        return false;
    }
    SDL_DisplayMode target;
    target.format = 0;
    target.w = mode.width;
    target.h = mode.height;
    target.refresh_rate = mode.refreshRate;
    target.driverdata = nullptr;
    SDL_SetWindowDisplayMode(mWindow, &target);
    SDL_SetWindowSize(mWindow, mode.width, mode.height);
    mWidth = mode.width;
    mHeight = mode.height;
    return true;
}

#endif

} // namespace gta2::hal