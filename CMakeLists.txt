cmake_minimum_required (VERSION 3.5)
project (gta2_port LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# The byte-matching VC6 build (build.py, Wine + VC6 toolchain) is the default only when
# compiling with MSVC 6.0 (MSVC_VERSION == 1200). Modern MSVC, GCC, Clang, etc. default
# to the modern portable build. Either can be forced with -DGTA2_MATCHING_BUILD=ON/OFF.
if(MSVC AND MSVC_VERSION EQUAL 1200)
    set(GTA2_MATCHING_BUILD_DEFAULT ON)
else()
    set(GTA2_MATCHING_BUILD_DEFAULT OFF)
endif()

option(GTA2_BACKEND "Target backend (SDL2, SDL3, HEADLESS)" "SDL2")
option(GTA2_MATCHING_BUILD "Build byte-matching VC6 executable (Wine only)" ${GTA2_MATCHING_BUILD_DEFAULT})

if(GTA2_MATCHING_BUILD)
    ENABLE_LANGUAGE(ASM_MASM)

    SET(CMAKE_WARN_VS6 CACHE BOOL OFF)

    add_subdirectory(3rdParty/GTA2Hax)

    add_library (mss32 SHARED
        Source/3rdParty/mss.h
        Source/3rdParty/mss.cpp
        )
    target_compile_definitions(mss32 PRIVATE BUILD_MSS)

    add_library (binkw32 SHARED
        Source/3rdParty/bink.h
        Source/3rdParty/bink.cpp
        )
    target_compile_definitions(binkw32 PRIVATE BUILD_BINK)

    #add_custom_command (
    #    TARGET mss32
    #    POST_BUILD
    #    COMMAND del $(TargetPath)
    #)

    # we build the code 3 times as:
    # a static lib where by we link it to an exe that is supposed to match the OG as closely as possible
    # a dll that contains the global vars as being exported and no functions exported
    # a dll that contains the global vars as being imported and all functions exported
    # this is so that for the hook dll we can redirect importing the vars from the 2nd copy of the dll
    # to the actual executable data/vars
    set(gta2_lib_src
      Source/Pool.hpp
      Source/crt_stubs.cpp
      Source/crt_stubs.hpp
      Source/Globals.cpp
      Source/Globals.hpp
      Source/3rdParty/mss.h
      Source/types.hpp
      Source/winmain.cpp
      Source/ScriptManager.hpp
      Source/ScriptManager.cpp
      Source/map_0x370.hpp
      Source/map_0x370.cpp
      Source/Light.hpp
      Source/Light.cpp
      Source/BitSet32.hpp
      Source/fix16.hpp
      Source/fix16.cpp
      Source/ang16.hpp
      Source/ang16.cpp
      Source/error.hpp
      Source/error.cpp
      Source/memory.hpp
      Source/memory.cpp
      Source/chunk.hpp
      Source/KeyboardLayout.hpp
      Source/KeyboardLayout.cpp
      Source/debug.hpp
      Source/debug.cpp
      Source/cSampleManager.hpp
      Source/cSampleManager.cpp
      Source/sound_obj.hpp
      Source/sound_obj.cpp
      Source/dma_video.hpp
      Source/dma_video.cpp
      Source/KanjiFont.hpp
      Source/KanjiFont.cpp
      Source/TextureCache.hpp
      Source/TextureCache.cpp
      Source/Draw.hpp
      Source/Draw.cpp
      Source/Bink.hpp
      Source/Bink.cpp
      Source/gbh_graphics.hpp
      Source/gbh_graphics.cpp
      Source/TextTable.hpp
      Source/TextTable.cpp
      Source/root_sound.hpp
      Source/root_sound.cpp
      Source/Player.hpp
      Source/Player.cpp
      Source/Frontend.hpp
      Source/Frontend.cpp
      Source/Game.hpp
      Source/Game.cpp
      Source/PlyDat.hpp
      Source/PlyDat.cpp
      Source/NetPlay.hpp
      Source/NetPlay.cpp
      Source/network_data.hpp
      Source/input.hpp
      Source/input.cpp
      Source/GameSession.hpp
      Source/GameSession.cpp
      Source/registry.hpp
      Source/registry.cpp
      Source/file.hpp
      Source/file.cpp
      Source/FpsCounter.hpp
      Source/FpsCounter.cpp
      Source/ProfilerTimer.hpp
      Source/ProfilerTimer.cpp
      Source/gtx_0x106C.hpp
      Source/gtx_0x106C.cpp
      Source/Function.hpp
      Source/Function.cpp
      Source/Camera.hpp
      Source/Camera.cpp
      Source/Ped.hpp
      Source/Ped.cpp
      Source/Car.hpp
      Source/Car.cpp
      Source/CarAI.hpp
      Source/CarAI.cpp
      Source/Car_10.hpp
      Source/Car_10.cpp
      Source/Fix16_Rect.hpp
      Source/Fix16_Rect.cpp
      Source/Fix16_Point.hpp
      Source/Fix16_Point.cpp
      Source/Fix16_Vec.hpp
      Source/Ped_List.hpp
      Source/Ped_List.cpp
      Source/PlayerCrimeStats.hpp
      Source/PlayerCrimeStats.cpp
      Source/SoundObject.hpp
      Source/SoundObject.cpp
      Source/BonusTracker.hpp
      Source/BonusTracker.cpp
      Source/Gang.hpp
      Source/Gang.cpp
      Source/TagGame.hpp
      Source/TagGame.cpp
      Source/CreditsText.hpp
      Source/CreditsText.cpp
      Source/CreditsLine.hpp
      Source/CreditsLine.cpp
      Source/ScriptThread.hpp
      Source/ScriptThread.cpp
      Source/ScriptGosubStack.hpp
      Source/ScriptGosubStack.cpp
      Source/miss2_xyz.hpp
      Source/InputRecorder_67F8B0.hpp
      Source/InputRecorder_67F8B0.cpp
      Source/Hud.hpp
      Source/Hud.cpp
      Source/MapRenderer.hpp
      Source/MapRenderer.cpp
      Source/SpriteRenderer.hpp
      Source/SpriteRenderer.cpp
      Source/FrameProfiler.hpp
      Source/FrameProfiler.cpp
      Source/ScriptStackFramePool.hpp
      Source/ScriptStackFramePool.cpp
      Source/sprite.hpp
      Source/sprite.cpp
      Source/char.hpp
      Source/char.cpp
      Source/Char_Pool.hpp
      Source/Char_Pool.cpp
      Source/collide.hpp
      Source/collide.cpp
      Source/ObjectDefinitions.hpp
      Source/ObjectDefinitions.cpp
      Source/Object_5C.hpp
      Source/Object_5C.cpp
      Source/PedRefTable.hpp
      Source/PedRefTable.cpp
      Source/TileAnimManager.hpp
      Source/TileAnimManager.cpp
      Source/WeaponManager.hpp
      Source/WeaponManager.cpp
      Source/DoorPool.hpp
      Source/DoorPool.cpp
      Source/AmbulanceService.hpp
      Source/AmbulanceService.cpp
      Source/PublicTransport.hpp
      Source/PublicTransport.cpp
      Source/TaxiList.hpp
      Source/TaxiList.cpp
      Source/TrafficLights.hpp
      Source/TrafficLights.cpp
      Source/RouteFinder.hpp
      Source/RouteFinder.cpp
      Source/PatrolRoutePool.hpp
      Source/PatrolRoutePool.cpp
      Source/PathFinder.hpp
      Source/PathFinder.cpp
      Source/CarInfoTable.hpp
      Source/CarInfoTable.cpp
      Source/ParticleManager.hpp
      Source/ParticleManager.cpp
      Source/Explosion.hpp
      Source/Explosion.cpp
      Source/Cranes.hpp
      Source/Cranes.cpp
      Source/Crushers.hpp
      Source/Crushers.cpp
      Source/Generators.hpp
      Source/Generators.cpp
      Source/EmergencyCrewPool.hpp
      Source/EmergencyCrewPool.cpp
      Source/Police.hpp
      Source/Police.cpp
      Source/LightPool.hpp
      Source/LightPool.cpp
      Source/Garage.hpp
      Source/Garage.cpp
      Source/CarChaseTaskTable.hpp
      Source/CarChaseTaskTable.cpp
      Source/ExplodingScore_100.hpp
      Source/ExplodingScore_100.cpp
      Source/CrimeReportQueue.hpp
      Source/CrimeReportQueue.cpp
      Source/Firefighters.hpp
      Source/Firefighters.cpp
      Source/CollisionSoundQueue.hpp
      Source/CollisionSoundQueue.cpp
      Source/SpriteGrid.hpp
      Source/SpriteGrid.cpp
      Source/NetUi.hpp
      Source/NetUi.cpp
      Source/PedGroup.hpp
      Source/PedGroup.cpp
      Source/rng.hpp
      Source/rng.cpp
      Source/PlayerScoreTracker.hpp
      Source/PlayerScoreTracker.cpp
      Source/RollingDigitCounter.hpp
      Source/RollingDigitCounter.cpp
      Source/enums.hpp
      Source/CarPhysics.hpp
      Source/CarPhysics.cpp
      Source/MissionCleanupList.hpp
      Source/MissionCleanupList.cpp
      Source/Object_8.hpp
      Source/Object_8.cpp
      Source/Object_8_Pool.hpp
      Source/Object_8_Pool.cpp
      Source/Object_3C_Pool.hpp
      Source/Object_3C_Pool.cpp
      Source/Object_3C.hpp
      Source/Object_3C.cpp
      Source/ObjectPool.hpp
      Source/ObjectPool.cpp
      Source/Particle_4C.hpp
      Source/Particle_4C.cpp
      Source/Police_38.hpp
      Source/Police_38.cpp
      Source/Door.hpp
      Source/Door.cpp
      Source/Weapon.hpp
      Source/Weapon.cpp
      Source/ImGuiDebug.hpp
      Source/ImGuiDebug.cpp
    )

    # global include dirs
    include_directories(Source)
    include_directories(${CMAKE_SOURCE_DIR})
    include_directories(3rdParty/gta2_re_compile_tools)

    # global lib dirs
    #link_directories(${CMAKE_SOURCE_DIR}/gta2_lib)

    set(dplay_libs
      ${CMAKE_SOURCE_DIR}/3rdParty/gta2_re_compile_tools/VC98/Lib/DPLAYX.lib)

    add_library (gta2_lib ${gta2_lib_src})
    target_compile_definitions(gta2_lib PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS IMGUI_DLL)
    target_link_libraries(gta2_lib)

    message(STATUS "${CMAKE_BINARY_DIR}=${${CMAKE_BINARY_DIR}}")

    add_library (gta2_dll_exports SHARED ${gta2_lib_src})
    target_compile_definitions(gta2_dll_exports PRIVATE EXPORT_VARS _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS IMGUI_DLL)
    target_link_libraries(gta2_dll_exports mss32 binkw32 Winmm ${dplay_libs} dear_imgui)

    add_library (gta2_dll_imports SHARED ${gta2_lib_src} Source/WinMainProxy.cpp)
    target_compile_definitions(gta2_dll_imports PRIVATE IMPORT_VARS EXPORT_FUNCS _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS IMGUI_DLL)
    target_link_libraries(gta2_dll_imports gta2_dll_exports mss32 binkw32 Winmm ${dplay_libs} dear_imgui)

    add_library(HookLoader SHARED
        Source/HookLoader.cpp
        Source/HookLoader.hpp
        Source/3rdParty/Detours/src/detours.cpp
        Source/3rdParty/Detours/src/disasm.cpp
        "Source/3rdParty/Manual-DLL-Loader/Source/Manual Loader/Loader.cpp"
    )
    target_compile_definitions(HookLoader PRIVATE DETOURS_X86 DETOURS_32BIT IMGUI_DLL)

    add_executable (ExePatcher Source/ExePatcher.cpp)
    if (${MSVC_VERSION} EQUAL 1200)
        target_link_libraries(ExePatcher libcmt.lib libcpmt.lib)
        set_target_properties(ExePatcher PROPERTIES COMPILE_FLAGS "/ML")
        set_target_properties(ExePatcher PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libc.lib /OPT:NOICF,NOREF")
    endif()

    add_executable (decomp_main WIN32
        test.cpp
        StdAfx.cpp
        resources.rc)
    target_link_libraries(decomp_main mss32 binkw32 gta2_lib ${dplay_libs} dear_imgui)

    if (${MSVC_VERSION} EQUAL 1200)
        target_link_libraries(decomp_main libc.lib libcp.lib)
        target_compile_definitions(decomp_main PRIVATE IMGUI_DLL)
        set_target_properties(decomp_main PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libc.lib")
    endif()

    if (${MSVC_VERSION} GREATER 1200)
        set_target_properties(decomp_main PROPERTIES
            LINK_FLAGS "/MAP:output.map /OPT:NOICF,NOREF /SAFESEH:NO"
        )
    else()
        set_target_properties(decomp_main PROPERTIES
            LINK_FLAGS "/MAP:output.map /OPT:NOICF,NOREF"
        )
    endif()

    if (${MSVC_VERSION} GREATER 1200)
        set_target_properties(gta2_dll_exports PROPERTIES COMPILE_FLAGS "/Gy")
        set_target_properties(gta2_dll_imports PROPERTIES COMPILE_FLAGS "/Gy")

        set_target_properties(gta2_dll_exports PROPERTIES LINK_FLAGS "/OPT:NOICF,NOREF /SAFESEH:NO /INCREMENTAL:NO" )
        set_target_properties(gta2_dll_imports PROPERTIES LINK_FLAGS "/OPT:NOICF,NOREF /SAFESEH:NO /INCREMENTAL:NO" )
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /OPT:NOICF,NOREF /SAFESEH:NO")
    else()
        set_target_properties(gta2_dll_exports PROPERTIES LINK_FLAGS "/OPT:NOICF,NOREF" )
        set_target_properties(gta2_dll_imports PROPERTIES LINK_FLAGS "/OPT:NOICF,NOREF" )
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /OPT:NOICF,NOREF")
    endif()


    add_executable (cl_proxy Source/cl_proxy.cpp)
    if (${MSVC_VERSION} EQUAL 1200)
        set_target_properties(cl_proxy PROPERTIES COMPILE_FLAGS "/ML")
        set_target_properties(cl_proxy PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libc.lib")
        target_link_libraries(cl_proxy  libcmt.lib libcpmt.lib)
    endif()

    if(${MSVC})
        include(cmake/vc6.cmake)
    endif ()
else()
    # ---------------------------------------------------------------------------
    # Modern portable build (GCC / Clang / MSVC 2019+ / Apple Clang / Emscripten)
    # ---------------------------------------------------------------------------
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include(cmake/Platform.cmake)

    # Find SDL packages
    if(GTA2_BACKEND STREQUAL "SDL2")
        find_package(SDL2 QUIET)
        if(NOT TARGET SDL2::SDL2)
            if(WIN32 OR ANDROID)
                include(FetchContent)
                set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
                FetchContent_Declare(
                    SDL2
                    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
                    GIT_TAG release-2.30.12
                )
                FetchContent_MakeAvailable(SDL2)
            else()
                find_package(SDL2 REQUIRED)
            endif()
        endif()
        add_compile_definitions(GTA2_USE_SDL2)
    elseif(GTA2_BACKEND STREQUAL "SDL3")
        find_package(SDL3 QUIET)
        if(NOT TARGET SDL3::SDL3)
            if(WIN32 OR ANDROID)
                include(FetchContent)
                FetchContent_Declare(
                    SDL3
                    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
                    GIT_TAG preview-3.1.6
                )
                FetchContent_MakeAvailable(SDL3)
            else()
                find_package(SDL3 REQUIRED)
            endif()
        endif()
        add_compile_definitions(GTA2_USE_SDL3)
    elseif(GTA2_BACKEND STREQUAL "HEADLESS")
        add_compile_definitions(GTA2_USE_HEADLESS)
    endif()

    # Include directories
    include_directories(
        Source
        Source/hal
    )

    # Collect game source files (excluding legacy Windows-only files and mock DLL stubs)
    file(GLOB_RECURSE GAME_SOURCES
        "Source/*.cpp"
    )
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/winmain.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/WinMainProxy.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/main.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/hal/.*")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/HookLoader.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/ExePatcher.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/cl_proxy.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/ImGuiDebug.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/NetPlay.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/NetUi.cpp")
    list(FILTER GAME_SOURCES EXCLUDE REGEX "Source/3rdParty/.*")

    add_library(gta2_core STATIC ${GAME_SOURCES})
    target_compile_definitions(gta2_core PUBLIC GTA2_MODERN_BUILD)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(gta2_core PRIVATE -fpermissive -Wno-write-strings)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(gta2_core PRIVATE -fpermissive -Wno-everything)
    elseif(MSVC)
        target_compile_options(gta2_core PRIVATE /W3 /permissive-)
    endif()

    # The HAL library (gta2_hal) is provided by Source/hal, added by a later task.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/hal/CMakeLists.txt")
        add_subdirectory(Source/hal)
    endif()

    # The game executable/library is wired in once Source/main.cpp exists.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/main.cpp" AND TARGET gta2_hal)
        if(ANDROID)
            add_library(main SHARED Source/main.cpp)
            target_link_libraries(main PRIVATE gta2_core gta2_hal log android)
            if(GTA2_BACKEND STREQUAL "SDL2")
                target_link_libraries(main PRIVATE SDL2::SDL2)
            elseif(GTA2_BACKEND STREQUAL "SDL3")
                target_link_libraries(main PRIVATE SDL3::SDL3)
            endif()
        else()
            set(GTA2_EXE_SOURCES Source/main.cpp)
            if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
                list(APPEND GTA2_EXE_SOURCES resources.rc)
            endif()
            add_executable(gta2 ${GTA2_EXE_SOURCES})
            target_link_libraries(gta2 PRIVATE gta2_core gta2_hal)

            if(GTA2_BACKEND STREQUAL "SDL2")
                target_link_libraries(gta2 PRIVATE SDL2::SDL2)
            elseif(GTA2_BACKEND STREQUAL "SDL3")
                target_link_libraries(gta2 PRIVATE SDL3::SDL3)
            endif()
        endif()
    endif()
endif()