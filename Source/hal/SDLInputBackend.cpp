#include "SDLInputBackend.hpp"

#include <SDL_events.h>
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
        int scancode = DikToSdlScancode(SdlScancodeToDik(ev.key.keysym.scancode));
        mKeysDownCurrent[scancode] = (ev.type == SDL_KEYDOWN);
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
    // Key states updated per event in HandleKeyboardEvent
}

void SDLInputBackend::Update() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                HandleKeyboardEvent(sdlEvent);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERAXISMOTION:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                HandleControllerEvent(sdlEvent);
                break;
            default:
                break;
        }
    }
    UpdateControllerStates();
}

bool SDLInputBackend::Initialize() {
    mControllers.clear();
    for (size_t i = 0; i < 256; ++i) {
        mKeysDownCurrent[i] = false;
        mKeysDownPrev[i] = false;
    }
    printf("[HAL] Initializing backend: Default\n");
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