#include "SDLWindowSystem.hpp"

#include <cstdio>

namespace gta2::hal {

// Force a 32-bit (8/8/8/8) colour framebuffer and log the visual depth the
// driver actually gives us. A 16-bit (RGB565) visual causes banding, darkened
// midtones and yellow/white colour casts (seen on radeonsi X sessions).
static void RequestAndLogFramebufferDepth(const char* tag) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int r = -1, g = -1, b = -1, a = -1, d = -1;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &d);
    std::fprintf(stderr, "[FB] %s framebuffer R=%d G=%d B=%d A=%d D=%d%s\n", tag, r, g, b, a, d,
                 (r == 8 && g == 8 && b == 8) ? "" : "  <-- NOT 8/8/8!");
}

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
    SDL_SetHint("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", getenv("GTA2_BYPASS_COMPOSITOR") ? "1" : "0");
    RequestAndLogFramebufferDepth("SDL3 pre-window");
    mWindow = SDL_CreateWindow(title ? title : "GTA2", width, height, flags);
    if (!mWindow) {
        return false;
    }
    mGLContext = SDL_GL_CreateContext(mWindow);
    // Do not sync swaps to the display refresh. SwapBuffers() already glFinish()es
    // so the front buffer is complete before present (fixes GPU-backed tearing).
    // Requesting vsync here breaks virtual/remote GL (e.g. Xrdp): SwapWindow then
    // presents the cleared back buffer on most frames, so the menu shows ~1 good
    // frame in 50 and black otherwise.
    SDL_GL_SetSwapInterval(0);
    RequestAndLogFramebufferDepth("SDL3 post-context");
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
        // Sync the GL pipeline before presenting so the front buffer contains every
        // draw command issued this frame. Without glFinish(), on GPU-backed GL
        // (e.g. radeonsi, Mesa 26.2.3) SDL_GL_SwapWindow can present a still-drawing
        // buffer — causing the menu to alternate between text-faded and background-only
        // frames. glFinish waits for all queued OpenGL commands to complete.
        if (void (*pFinish)(void) = reinterpret_cast<void (*)(void)>(SDL_GL_GetProcAddress("glFinish"))) {
            pFinish();
        }
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
    SDL_SetHint("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", getenv("GTA2_BYPASS_COMPOSITOR") ? "1" : "0");
    RequestAndLogFramebufferDepth("SDL2 pre-window");
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
    // See SDL3 path: glFinish in SwapBuffers prevents tearing; vsync breaks
    // virtual/remote GL (Xrdp) by presenting the cleared back buffer.
    SDL_GL_SetSwapInterval(0);
    RequestAndLogFramebufferDepth("SDL2 post-context");
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
        // Sync the GL pipeline before presenting so the front buffer contains every
        // draw command issued this frame. Without glFinish(), on GPU-backed GL
        // (e.g. radeonsi, Mesa 26.2.3) SDL_GL_SwapWindow can present a still-drawing
        // buffer — causing the menu to alternate between text-faded and background-only
        // frames. glFinish waits for all queued OpenGL commands to complete.
        if (void (*pFinish)(void) = reinterpret_cast<void (*)(void)>(SDL_GL_GetProcAddress("glFinish"))) {
            pFinish();
        }
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