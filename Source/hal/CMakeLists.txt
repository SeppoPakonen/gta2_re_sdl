add_library(gta2_hal STATIC
    HalContext.cpp
    VfsFileSystem.cpp
    IniConfigBackend.cpp
    SDLWindowSystem.cpp
    SDLEventSystem.cpp
    SDLTimeSystem.cpp
    OpenGLRenderer.cpp
    SDL3GpuRenderer.cpp
)
target_include_directories(gta2_hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})