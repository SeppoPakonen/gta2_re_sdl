#include "VirtualTouchpad.hpp"

#include <cmath>

namespace gta2::hal {

static constexpr f32 kStickRadius = 0.22f;
static constexpr f32 kButtonRadius = 0.10f;
static constexpr f32 kStickDeadzone = 0.15f;

inline bool InButtonRegion(f32 normX, f32 normY, f32 btnX, f32 btnY, f32 r)
{
    f32 dx = normX - btnX;
    f32 dy = normY - btnY;
    f32 d2 = dx * dx + dy * dy;
    return d2 <= (r * r);
}

void VirtualTouchpad::HandleTouchDown(f32 normX, f32 normY, u32 touchId)
{
    (void)normX;
    (void)normY;
    (void)touchId;
    s32 idx = FindFreeTouchIndex();
    if (idx < 0) return;

    mTouchX[idx] = normX;
    mTouchY[idx] = normY;
    mTouchId[idx] = touchId;
    mTouchActive[idx] = true;
    mActiveTouchCount++;

    bool xRight = normX >= 0.5f;
    if (xRight) {
        mStickTouchActive = false;
        RecomputeButtonMask();
    } else {
        mStickTouchActive = true;
        mStickTouchId = touchId;
        UpdateStickFromTouch(normX, normY);
    }
}

void VirtualTouchpad::HandleTouchMotion(f32 normX, f32 normY, u32 touchId)
{
    s32 idx = FindTouchIndex(touchId);
    if (idx < 0) return;

    mTouchX[idx] = normX;
    mTouchY[idx] = normY;

    if (mStickTouchActive && mTouchId[idx] == mStickTouchId) {
        UpdateStickFromTouch(normX, normY);
    } else {
        RecomputeButtonMask();
    }
}

void VirtualTouchpad::HandleTouchUp(f32 normX, f32 normY, u32 touchId)
{
    s32 idx = FindTouchIndex(touchId);
    if (idx < 0) return;

    mTouchActive[idx] = false;
    mActiveTouchCount--;

    if (mStickTouchActive && mTouchId[idx] == mStickTouchId) {
        mStickTouchActive = false;
        mStickTouchId = 0;
        mStickX = 0.0f;
        mStickY = 0.0f;
    }

    if (!mStickTouchActive || (idx >= 0 && !mTouchActive[idx])) {
        RecomputeButtonMask();
    }
}

void VirtualTouchpad::GetState(u32& outButtonMask, f32& outStickX, f32& outStickY) const
{
    outButtonMask = mButtonMask;
    outStickX = mStickX;
    outStickY = mStickY;
}

void VirtualTouchpad::RenderOverlay(IRenderer* pRenderer, s32 screenW, s32 screenH)
{
    (void)pRenderer;
    (void)screenW;
    (void)screenH;
}

s32 VirtualTouchpad::FindTouchIndex(u32 touchId) const
{
    for (u32 i = 0; i < kMaxTouches; ++i) {
        if (mTouchActive[i] && mTouchId[i] == touchId) {
            return static_cast<s32>(i);
        }
    }
    return -1;
}

s32 VirtualTouchpad::FindFreeTouchIndex() const
{
    for (u32 i = 0; i < kMaxTouches; ++i) {
        if (!mTouchActive[i]) {
            return static_cast<s32>(i);
        }
    }
    return -1;
}

void VirtualTouchpad::UpdateStickFromTouch(f32 normX, f32 normY)
{
    f32 dx = normX - mStickCenterX;
    f32 dy = normY - mStickCenterY;
    f32 distSq = dx * dx + dy * dy;
    if (distSq <= (kStickRadius * kStickRadius)) {
        f32 dist = sqrtf(distSq);
        f32 len = dist / kStickRadius;
        f32 scaled = (len > 1.0f) ? 1.0f : ((len < 0.0f) ? 0.0f : len);
        mStickX = scaled;
        mStickY = scaled;
    } else {
        f32 len = sqrtf(distSq) / kStickRadius;
        if (len > 1.0f) {
            f32 nx = (normX - mStickCenterX) / kStickRadius;
            f32 ny = (normY - mStickCenterY) / kStickRadius;
            f32 d = sqrtf(nx * nx + ny * ny);
            if (d > kStickRadius) {
                f32 scale = kStickRadius / d;
                mStickX = nx * scale;
                mStickY = ny * scale;
            }
        }
    }
}

void VirtualTouchpad::UpdateButtonsFromTouch(f32 normX, f32 normY)
{
    (void)normX;
    (void)normY;
}

void VirtualTouchpad::RecomputeButtonMask()
{
    mButtonMask = 0;
    if (!mEnabled) return;

    for (u32 i = 0; i < kMaxTouches; ++i) {
        if (!mTouchActive[i]) continue;
        f32 nx = mTouchX[i];
        f32 ny = mTouchY[i];

        if (InButtonRegion(nx, ny, 0.82f, 0.64f, kButtonRadius)) {
            mButtonMask |= BTN_ACCELERATE;
        } else if (InButtonRegion(nx, ny, 0.82f, 0.80f, kButtonRadius)) {
            mButtonMask |= BTN_BRAKE;
        } else if (InButtonRegion(nx, ny, 0.90f, 0.72f, kButtonRadius)) {
            mButtonMask |= BTN_ENTER_EXIT;
        } else if (InButtonRegion(nx, ny, 0.74f, 0.72f, kButtonRadius)) {
            mButtonMask |= BTN_ATTACK;
        } else if (InButtonRegion(nx, ny, 0.82f, 0.56f, kButtonRadius)) {
            mButtonMask |= BTN_HORN;
        }
    }
}

} // namespace gta2::hal