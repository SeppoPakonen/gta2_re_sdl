add_library(gta2_hal STATIC
    HalContext.cpp
    VfsFileSystem.cpp
    IniConfigBackend.cpp
    SDLWindowSystem.cpp
    SDLEventSystem.cpp
    SDLTimeSystem.cpp
    OpenGLRenderer.cpp
    SDL3GpuRenderer.cpp
    PaletteManager.cpp
    AudioContainerParser.cpp
    SDLAudioBackend.cpp
    AudioMixer3D.cpp
    VirtualTouchpad.cpp
    SDLInputBackend.cpp
    InputKeyMap.cpp
    FFmpegMoviePlayer.cpp
)
target_include_directories(gta2_hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(SDL2_LIBRARIES)
    target_link_libraries(gta2_hal PUBLIC ${SDL2_LIBRARIES})
    if(SDL2_LIBRARY_DIRS)
        target_link_directories(gta2_hal PUBLIC ${SDL2_LIBRARY_DIRS})
    endif()
    if(SDL2_INCLUDE_DIRS)
        target_include_directories(gta2_hal PUBLIC ${SDL2_INCLUDE_DIRS})
    endif()
elseif(TARGET SDL2::SDL2)
    target_link_libraries(gta2_hal PUBLIC SDL2::SDL2)
elseif(TARGET SDL3::SDL3)
    target_link_libraries(gta2_hal PUBLIC SDL3::SDL3)
endif()

# libav* (ffmpeg) backs the intro movie player. Optional: without it the movie
# player falls back to NullMoviePlayer.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG QUIET libavformat libavcodec libavutil libswscale libswresample)
endif()
if(FFMPEG_FOUND)
    target_include_directories(gta2_hal PUBLIC ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(gta2_hal PUBLIC ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(gta2_hal PUBLIC ${FFMPEG_LIBRARIES})
    target_compile_definitions(gta2_hal PUBLIC GTA2_HAVE_FFMPEG=1)
    message(STATUS "GTA2: intro movie via ffmpeg (libavformat/libavcodec)")
else()
    message(STATUS "GTA2: ffmpeg not found, intro movie disabled")
endif()
