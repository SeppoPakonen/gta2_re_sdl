# GTA2 SDL Port (Cross-Platform Modern Engine)

An open-source, cross-platform port of Grand Theft Auto 2 based on reverse-engineered source code. This project modernizes the original Windows 95/98/DirectX 7 engine to run natively on **Linux**, **Windows**, **Android**, **FreeBSD**, and **WebAssembly (HTML5)** using **SDL2** (with SDL3 support), OpenGL, and an extensible Hardware Abstraction Layer (HAL).

The default build target is the **Modern SDL2 Port** (`GTA2_MODERN_BUILD=ON`), replacing legacy Win32, DirectDraw, Direct3D, DirectInput, and Miles Sound System dependencies with clean, portable abstractions.

---

## 🎮 Game Assets & Legal Notice

This repository contains **only the open-source reconstructed game engine code**. It does **not** include or distribute any copyrighted Rockstar Games assets (textures, maps, scripts, sounds, or music). No binary bundles or asset packs are distributed.

To play the game, you must provide your own legitimate copy of GTA2 assets:

- **Official Free Release**: Rockstar Games previously released GTA2 as a free, DRM-free digital download (part of the *Rockstar Classics* series). Copies of this installer are archived and legally downloadable at [Archive.org: GTA2 Rockstar Classics](https://archive.org/details/gta2_rockstar-classics).
- **Installation via Wine on Linux**:
  You can run the official installer inside Wine:
  ```bash
  wine GTA2.exe
  ```
- **Automatic Asset Detection**:
  The modern SDL executable automatically detects and mounts assets from the standard Wine default installation directory:
  ```
  $HOME/.wine/drive_c/Program Files (x86)/Rockstar Games/GTA2
  ```
  (or `$HOME/.wine/drive_c/Program Files/Rockstar Games/GTA2`).
  Alternatively, you can point the `GTA2_ROOT` environment variable to your game directory, or place a symlink/copy of the `data/` folder into your working directory:
  ```bash
  export GTA2_ROOT="$HOME/.wine/drive_c/Program Files (x86)/Rockstar Games/GTA2"
  ```

---

## 🛠️ Building from Source

### Linux (Native SDL2 / OpenGL)

#### Prerequisites
Install CMake, a modern C++17 compiler (GCC 10+ or Clang 12+), and SDL2 development headers:

- **Debian / Ubuntu / Mint**:
  ```bash
  sudo apt update
  sudo apt install -y build-essential cmake libsdl2-dev libgl1-mesa-dev gdb
  ```
- **Arch Linux / Manjaro**:
  ```bash
  sudo pacman -S --needed base-devel cmake sdl2 mesa gdb
  ```
- **Fedora / RHEL**:
  ```bash
  sudo dnf install -y gcc-c++ cmake SDL2-devel mesa-libGL-devel gdb
  ```

#### Compile
```bash
# Configure modern SDL2 release build (default)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DGTA2_BACKEND=SDL2

# Build the executable
cmake --build build -j$(nproc)
```

The compiled binary will be placed at `build/gta2`.

#### Debug Build & GDB Tracing
```bash
cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g3 -O0 -fno-omit-frame-pointer" -DGTA2_BACKEND=SDL2
cmake --build build_debug -j$(nproc)

# Run with GDB
gdb ./build_debug/gta2
```

---

### Windows

#### Option A: MSYS2 / MinGW-w64 (Recommended for CLI / GCC)
1. Open the **MSYS2 MinGW 64-bit** shell (`mingw64.exe`).
2. Install the toolchain and dependencies:
   ```bash
   pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-SDL2
   ```
3. Build the project:
   ```bash
   cmake -B build_win -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DGTA2_BACKEND=SDL2
   cmake --build build_win
   ```
   The resulting executable will be `build_win/gta2.exe`.

#### Option B: Microsoft Visual Studio (MSVC 2019 / 2022)
1. Open the project folder in Visual Studio or use the Developer Command Prompt:
   ```cmd
   cmake -B build_msvc -DCMAKE_BUILD_TYPE=Release -DGTA2_BACKEND=SDL2
   cmake --build build_msvc --config Release
   ```
   *Note: SDL2 is automatically acquired via CMake FetchContent if not found locally.*

---

### Other Platforms
- **Android**: Gradle + NDK CMake build located in `android/` with full touch overlay support.
- **WebAssembly (Emscripten)**: Run `scripts/build_wasm.sh` to generate WebGL/WebAudio HTML5 playable artifacts.
- **FreeBSD**: Builds cleanly using FreeBSD ports Clang and `devel/sdl20`.

---

## 🕹️ Controls

| Action | Keyboard | Gamepad (Xbox/PlayStation) |
|---|---|---|
| **Steer / Move** | Arrow Keys / WASD | Left Analog Stick / D-Pad |
| **Accelerate / Run** | Up Arrow / W | Right Trigger (RT) / Button A |
| **Brake / Reverse** | Down Arrow / S | Left Trigger (LT) / Button X |
| **Attack / Shoot** | Left Ctrl / Space | Right Bumper (RB) / Button B |
| **Enter / Exit Car** | Enter / F | Button Y (Triangle) |
| **Next / Prev Weapon** | Z / X | D-Pad Left / Right |
| **Pause Menu** | Escape | Start / Menu Button |

---

## 🏛️ Architecture & HAL

All platform subsystems are abstracted behind modern pure-virtual interfaces in `Source/hal/`:
- `IWindowSystem` & `IEventSystem`: SDL2 window management and event pump.
- `IRenderer`: OpenGL 2.1 / GLES 2.0 shader-based tile & sprite renderer with dynamic 8-bit to 32-bit palette conversions (`PaletteManager`).
- `IAudioBackend`: Multi-voice PCM mixing engine with 3D spatial distance attenuation and stereo panning.
- `IInputBackend`: SDL scancode to DirectInput DIK mapping and analog controller handling.
- `IFileSystem`: Case-insensitive VFS path resolver with automatic search path discovery.

---

## 📜 Legacy Byte-Matching Build (Archival)
For researchers interested in the historical byte-matching VC6 Windows 98 executable verification:
```bash
# Requires Visual C++ 6.0 toolchain & Wine environment
python3 vc6_setup.py
python3 build.py
```
*Note: Byte-matching is disabled by default and only activates when compiling with MSVC 6 or `-DGTA2_MATCHING_BUILD=ON`.*
