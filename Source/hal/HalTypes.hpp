#pragma once

#include <stdint.h>
#include <stddef.h>

namespace gta2::hal {

using u8  = uint8_t;
using s8  = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;
using f32 = float;
using f64 = double;

// Vertex format matching legacy GTA2 Vert struct
struct HalVertex {
    f32 x, y, z, w; // Projected / camera coordinates
    u32 diff;       // Diffuse colour (ARGB 32-bit)
    u32 spec;       // Specular colour
    f32 u, v;       // Normalized texture coordinates
};

// 2D display mode description
struct DisplayMode {
    s32 width;
    s32 height;
    s32 refreshRate;
    s32 bitsPerPixel;
};

// Texture creation / registration descriptor
struct TextureDesc {
    u16 width;
    u16 height;
    u8  bitsPerPixel;   // 8 (paletted) or 16 (RGB565 / ARGB1555) or 32 (RGBA8888)
    const void* pPixels;
    const u16* pPalette;     // 256-entry 16-bit RGB565 palette for 8-bit textures
    const u32* pPaletteRGBA; // 256-entry 32-bit RGBA palette
    bool bTransparent;
};

// Handle identifying a registered hardware texture
using TextureHandle = u32;
constexpr TextureHandle kInvalidTextureHandle = 0;

// Render primitive flags (matching GTA2 engine draw flags)
enum RenderFlags : u32 {
    RENDER_NONE             = 0,
    RENDER_ALPHA_BLEND      = 0x180,
    RENDER_ADDITIVE         = 0x200,
};

// Engine events
enum class EventType {
    None,
    Quit,
    WindowClose,
    WindowResize,
    WindowFocusGained,
    WindowFocusLost,
    KeyDown,
    KeyUp,
    GamepadButtonDown,
    GamepadButtonUp,
    GamepadAxisMotion,
    TouchDown,
    TouchUp,
    TouchMotion,
};

struct Event {
    EventType type;
    union {
        struct { s32 width, height; } windowResize;
        struct { u32 scancode; bool repeat; } key;
        struct { u32 button; u8 gamepadIndex; } gamepadButton;
        struct { u8 axis; f32 value; u8 gamepadIndex; } gamepadAxis;
        struct { f32 x, y; u32 touchId; } touch;
    };
};

} // namespace gta2::hal