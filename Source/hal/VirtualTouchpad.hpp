#pragma once

#include "HalTypes.hpp"
#include "IRenderer.hpp"

namespace gta2::hal {

class VirtualTouchpad {
  private:
    static constexpr u32 kMaxTouches = 10;
    static constexpr f32 kStickRadius = 0.22f;  // normalized screen fraction
    static constexpr f32 kButtonRadius = 0.10f; // normalized screen fraction
    static constexpr f32 kStickDeadzone = 0.15f;

    bool mEnabled = false;
    f32 mStickX = 0.0f;
    f32 mStickY = 0.0f;
    u32 mButtonMask = 0;

    enum VirtualButton : u32 {
        BTN_ACCELERATE = 0x01,
        BTN_BRAKE      = 0x02,
        BTN_ENTER_EXIT = 0x04,
        BTN_ATTACK     = 0x08,
        BTN_HORN       = 0x10,
    };

    f32 mStickCenterX = 0.18f;
    f32 mStickCenterY = 0.72f;

    f32 mTouchX[kMaxTouches] = {};
    f32 mTouchY[kMaxTouches] = {};
    u32 mTouchId[kMaxTouches] = {};
    bool mTouchActive[kMaxTouches] = {};
    u32 mActiveTouchCount = 0;
    u32 mStickTouchId = 0;
    bool mStickTouchActive = false;

    s32 FindTouchIndex(u32 touchId) const;
    s32 FindFreeTouchIndex() const;
    void UpdateStickFromTouch(f32 normX, f32 normY);
    void UpdateButtonsFromTouch(f32 normX, f32 normY);
    void RecomputeButtonMask();

  public:
    VirtualTouchpad() = default;

    void SetEnabled(bool enabled) { mEnabled = enabled; }
    bool IsEnabled() const { return mEnabled; }

    void HandleTouchDown(f32 normX, f32 normY, u32 touchId);
    void HandleTouchMotion(f32 normX, f32 normY, u32 touchId);
    void HandleTouchUp(f32 normX, f32 normY, u32 touchId);

    void GetState(u32& outButtonMask, f32& outStickX, f32& outStickY) const;
    void RenderOverlay(IRenderer* pRenderer, s32 screenW, s32 screenH);
};

} // namespace gta2::hal