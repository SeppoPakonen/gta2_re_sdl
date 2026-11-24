#pragma once

#include "IInputBackend.hpp"
#include "InputKeyMap.hpp"

#if defined(GTA2_USE_SDL3)
    #include <SDL3/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

#include <vector>
#include <array>

namespace gta2::hal {

class SDLInputBackend : public IInputBackend {
  private:
    static constexpr u32 kMaxControllers = 4;
    static constexpr f32 kDeadzone = 0.15f;

#if defined(GTA2_USE_SDL3)
    using GamepadHandle = SDL_Gamepad;
    #define GTA2_GAMEPAD_AXIS_LEFT_X   SDL_GAMEPAD_AXIS_LEFT_X
    #define GTA2_GAMEPAD_AXIS_LEFT_Y   SDL_GAMEPAD_AXIS_LEFT_Y
    #define GTA2_GAMEPAD_AXIS_RIGHT_X  SDL_GAMEPAD_AXIS_RIGHT_X
    #define GTA2_GAMEPAD_AXIS_RIGHT_Y  SDL_GAMEPAD_AXIS_RIGHT_Y
    static inline GamepadHandle* OpenGamepad(int which) { return SDL_GamepadOpen(which); }
    static inline void CloseGamepad(GamepadHandle* handle) { SDL_GamepadClose(handle); }
    static inline bool IsGamepadConnected(GamepadHandle* handle) { return handle && SDL_GamepadConnected(handle); }
    static inline bool GetGamepadAxis(GamepadHandle* handle, int axis, int* value) { return SDL_GamepadGetAxis(handle, axis, value); }
#else
    using GamepadHandle = SDL_GameController;
    #define GTA2_GAMEPAD_AXIS_LEFT_X   SDL_CONTROLLER_AXIS_LEFTX
    #define GTA2_GAMEPAD_AXIS_LEFT_Y   SDL_CONTROLLER_AXIS_LEFTY
    #define GTA2_GAMEPAD_AXIS_RIGHT_X  SDL_CONTROLLER_AXIS_RIGHTX
    #define GTA2_GAMEPAD_AXIS_RIGHT_Y  SDL_CONTROLLER_AXIS_RIGHTY
    static inline GamepadHandle* OpenGamepad(int which) { return SDL_GameControllerOpen(which); }
    static inline void CloseGamepad(GamepadHandle* handle) { SDL_GameControllerClose(handle); }
    static inline bool IsGamepadConnected(GamepadHandle* handle) { return handle && SDL_GameControllerGetAttached(handle); }
    static inline bool GetGamepadAxis(GamepadHandle* handle, int axis, int* value) { 
        *value = SDL_GameControllerGetAxis(handle, static_cast<SDL_GameControllerAxis>(axis)); 
        return true; 
    }
#endif

    struct ControllerState {
        GamepadHandle* handle = nullptr;
        std::array<bool, 16> buttonsDown = {};
        std::array<bool, 16> buttonsPrev = {};
        std::array<f32, 6> axes = {};
    };

    std::vector<ControllerState> mControllers;
    std::array<bool, 256> mKeysDownCurrent = {};
    std::array<bool, 256> mKeysDownPrev = {};

    void HandleControllerEvent(const SDL_Event& ev);
    void HandleKeyboardEvent(const SDL_Event& ev);
    void UpdateControllerStates();
    void UpdateKeyboardState();

  public:
    SDLInputBackend() = default;
    ~SDLInputBackend() override { Shutdown(); }

    bool Initialize() override;
    void Shutdown() override;
    void Update() override;

    bool IsKeyDown(u32 dikScancode) const override;
    bool WasKeyPressed(u32 dikScancode) const override;
    bool WasKeyReleased(u32 dikScancode) const override;

    bool IsGamepadConnected(u8 gamepadIndex) const override;
    bool IsGamepadButtonDown(u8 gamepadIndex, u32 button) const override;
    f32  GetGamepadAxis(u8 gamepadIndex, u8 axis) const override;

    void SetVirtualTouchLayoutEnabled(bool enabled) override { (void)enabled; }
    bool GetVirtualGamepadState(u32& virtualButtonMask, f32& axisX, f32& axisY) const override {
        virtualButtonMask = 0;
        axisX = 0.0f;
        axisY = 0.0f;
        return false;
    }
};

} // namespace gta2::hal