#include "SDLInputBackend.hpp"
#include "types.hpp"

#include <cstdio>

namespace gta2::hal {

void SDLInputBackend::HandleControllerEvent(const SDL_Event& ev) {
    if (ev.type == SDL_CONTROLLERDEVICEADDED) {
        GamepadHandle* pad = OpenGamepad(ev.cdevice.which);
        if (pad) {
            mControllers.push_back({pad, {}, {}, {}});
        }
    } else if (ev.type == SDL_CONTROLLERDEVICEREMOVED) {
        for (auto it = mControllers.rbegin(); it != mControllers.rend(); ++it) {
            if (it->handle && IsGamepadConnected(it->handle)) {
                CloseGamepad(it->handle);
                it->handle = nullptr;
                break;
            }
        }
    }
}

void SDLInputBackend::HandleKeyboardEvent(const SDL_Event& ev) {
    if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
        u32 dik = SdlScancodeToDik(ev.key.keysym.scancode);
        if (dik < 256) {
            mKeysDownCurrent[dik] = (ev.type == SDL_KEYDOWN);
        }
    }
}

void SDLInputBackend::UpdateControllerStates() {
    for (auto& st : mControllers) {
        if (st.handle && IsGamepadConnected(st.handle)) {
            int ax0 = 0, ay0 = 0;
            int ax1 = 0, ay1 = 0;
            GetGamepadAxis(st.handle, GTA2_GAMEPAD_AXIS_LEFT_X, &ax0);
            GetGamepadAxis(st.handle, GTA2_GAMEPAD_AXIS_LEFT_Y, &ay0);
            GetGamepadAxis(st.handle, GTA2_GAMEPAD_AXIS_RIGHT_X, &ax1);
            GetGamepadAxis(st.handle, GTA2_GAMEPAD_AXIS_RIGHT_Y, &ay1);
            auto normalize = [](int val) -> f32 {
                f32 v = static_cast<f32>(val) / 32767.0f;
                if (v > kDeadzone) v -= kDeadzone;
                else if (v < -kDeadzone) v += kDeadzone;
                else v = 0.0f;
                return v;
            };
            st.axes[0] = normalize(ax0);
            st.axes[1] = normalize(ay0);
            st.axes[2] = normalize(ax1);
            st.axes[3] = normalize(ay1);
        }
    }
}

void SDLInputBackend::UpdateKeyboardState() {
    for (size_t i = 0; i < 256; ++i) {
        mKeysDownPrev[i] = mKeysDownCurrent[i];
    }
    int numKeys = 0;
    const Uint8* state = SDL_GetKeyboardState(&numKeys);
    if (state && numKeys > 0) {
        for (u32 dik = 0; dik < 256; ++dik) {
            u32 sdlScancode = DikToSdlScancode(dik);
            if (sdlScancode < static_cast<u32>(numKeys)) {
                mKeysDownCurrent[dik] = (state[sdlScancode] != 0);
            } else {
                mKeysDownCurrent[dik] = false;
            }
        }
    }
}

void SDLInputBackend::Update() {
    UpdateKeyboardState();
    UpdateControllerStates();
}

bool SDLInputBackend::Initialize() {
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    mControllers.clear();
    for (size_t i = 0; i < 256; ++i) {
        mKeysDownCurrent[i] = false;
        mKeysDownPrev[i] = false;
    }
    GTA2_VLOG("[HAL] Initializing backend: Default\n");
    return true;
}

void SDLInputBackend::Shutdown() {
    for (auto& st : mControllers) {
        if (st.handle) {
            CloseGamepad(st.handle);
            st.handle = nullptr;
        }
    }
    mControllers.clear();
    for (size_t i = 0; i < 256; ++i) {
        mKeysDownCurrent[i] = false;
        mKeysDownPrev[i] = false;
    }
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

bool SDLInputBackend::IsKeyDown(u32 dikScancode) const {
    return mKeysDownCurrent[dikScancode];
}

bool SDLInputBackend::WasKeyPressed(u32 dikScancode) const {
    return mKeysDownPrev[dikScancode];
}

bool SDLInputBackend::WasKeyReleased(u32 dikScancode) const {
    return !mKeysDownCurrent[dikScancode];
}

bool SDLInputBackend::IsGamepadConnected(u8 gamepadIndex) const {
    u32 idx = 0;
    for (const auto& st : mControllers) {
        if (st.handle && IsGamepadConnected(st.handle)) {
            if (idx == gamepadIndex) return true;
            idx++;
        }
    }
    return false;
}

bool SDLInputBackend::IsGamepadButtonDown(u8 gamepadIndex, u32 button) const {
    u32 idx = 0;
    for (const auto& st : mControllers) {
        if (st.handle && IsGamepadConnected(st.handle)) {
            if (idx == gamepadIndex && button < 16 && st.buttonsDown[button]) {
                return true;
            }
            idx++;
        }
    }
    return false;
}

f32 SDLInputBackend::GetGamepadAxis(u8 gamepadIndex, u8 axis) const {
    u32 idx = 0;
    for (const auto& st : mControllers) {
        if (st.handle && IsGamepadConnected(st.handle)) {
            if (idx == gamepadIndex && axis >= 0 && axis < 6) {
                switch (axis) {
                    case 0: return st.axes[0];
                    case 1: return st.axes[1];
                    case 2: return st.axes[2];
                    case 3: return st.axes[3];
                    case 4: return st.axes[4];
                    case 5: return st.axes[5];
                    default: return 0.0f;
                }
            }
            idx++;
        }
    }
    return 0.0f;
}

} // namespace gta2::hal