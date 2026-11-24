#pragma once
#include "HalTypes.hpp"

namespace gta2::hal {

class IInputBackend {
public:
    virtual ~IInputBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update() = 0;

    // Keyboard (scancode follows DirectInput DIK values for internal game compatibility)
    virtual bool IsKeyDown(u32 dikScancode) const = 0;
    virtual bool WasKeyPressed(u32 dikScancode) const = 0;
    virtual bool WasKeyReleased(u32 dikScancode) const = 0;

    // Gamepad
    virtual bool IsGamepadConnected(u8 gamepadIndex) const = 0;
    virtual bool IsGamepadButtonDown(u8 gamepadIndex, u32 button) const = 0;
    virtual f32  GetGamepadAxis(u8 gamepadIndex, u8 axis) const = 0;

    // Virtual Touch Gamepad (Mobile / Browser)
    virtual void SetVirtualTouchLayoutEnabled(bool enabled) = 0;
    virtual bool GetVirtualGamepadState(u32& virtualButtonMask, f32& axisX, f32& axisY) const = 0;
};

} // namespace gta2::hal