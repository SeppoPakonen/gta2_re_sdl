#pragma once

#include "HalTypes.hpp"

#if defined(GTA2_USE_SDL3)
    #include <SDL3/SDL_scancode.h>
#else
    #include <SDL2/SDL_scancode.h>
#endif

namespace gta2::hal {

// DirectInput 7 Scancodes
constexpr u32 GTA2_DIK_ESCAPE       = 0x01;
constexpr u32 GTA2_DIK_1            = 0x02;
constexpr u32 GTA2_DIK_2            = 0x03;
constexpr u32 GTA2_DIK_3            = 0x04;
constexpr u32 GTA2_DIK_4            = 0x05;
constexpr u32 GTA2_DIK_5            = 0x06;
constexpr u32 GTA2_DIK_6            = 0x07;
constexpr u32 GTA2_DIK_7            = 0x08;
constexpr u32 GTA2_DIK_8            = 0x09;
constexpr u32 GTA2_DIK_9            = 0x0A;
constexpr u32 GTA2_DIK_0            = 0x0B;
constexpr u32 GTA2_DIK_MINUS        = 0x0C;
constexpr u32 GTA2_DIK_EQUALS       = 0x0D;
constexpr u32 GTA2_DIK_BACKSPACE    = 0x0E;
constexpr u32 GTA2_DIK_TAB          = 0x0F;
constexpr u32 GTA2_DIK_Q            = 0x10;
constexpr u32 GTA2_DIK_W            = 0x11;
constexpr u32 GTA2_DIK_E            = 0x12;
constexpr u32 GTA2_DIK_R            = 0x13;
constexpr u32 GTA2_DIK_T            = 0x14;
constexpr u32 GTA2_DIK_Y            = 0x15;
constexpr u32 GTA2_DIK_U            = 0x16;
constexpr u32 GTA2_DIK_I            = 0x17;
constexpr u32 GTA2_DIK_O            = 0x18;
constexpr u32 GTA2_DIK_P            = 0x19;
constexpr u32 GTA2_DIK_LBRACKET     = 0x1A;
constexpr u32 GTA2_DIK_RBRACKET     = 0x1B;
constexpr u32 GTA2_DIK_RETURN       = 0x1C;
constexpr u32 GTA2_DIK_LCONTROL     = 0x1D;
constexpr u32 GTA2_DIK_A            = 0x1E;
constexpr u32 GTA2_DIK_S            = 0x1F;
constexpr u32 GTA2_DIK_D            = 0x20;
constexpr u32 GTA2_DIK_F            = 0x21;
constexpr u32 GTA2_DIK_G            = 0x22;
constexpr u32 GTA2_DIK_H            = 0x23;
constexpr u32 GTA2_DIK_J            = 0x24;
constexpr u32 GTA2_DIK_K            = 0x25;
constexpr u32 GTA2_DIK_L            = 0x26;
constexpr u32 GTA2_DIK_SEMICOLON    = 0x27;
constexpr u32 GTA2_DIK_APOSTROPHE   = 0x28;
constexpr u32 GTA2_DIK_GRAVE        = 0x29;
constexpr u32 GTA2_DIK_LSHIFT       = 0x2A;
constexpr u32 GTA2_DIK_BACKSLASH    = 0x2B;
constexpr u32 GTA2_DIK_Z            = 0x2C;
constexpr u32 GTA2_DIK_X            = 0x2D;
constexpr u32 GTA2_DIK_C            = 0x2E;
constexpr u32 GTA2_DIK_V            = 0x2F;
constexpr u32 GTA2_DIK_B            = 0x30;
constexpr u32 GTA2_DIK_N            = 0x31;
constexpr u32 GTA2_DIK_M            = 0x32;
constexpr u32 GTA2_DIK_COMMA        = 0x33;
constexpr u32 GTA2_DIK_PERIOD       = 0x34;
constexpr u32 GTA2_DIK_SLASH        = 0x35;
constexpr u32 GTA2_DIK_RSHIFT       = 0x36;
constexpr u32 GTA2_DIK_NUMPADSTAR   = 0x37;
constexpr u32 GTA2_DIK_LALT         = 0x38;
constexpr u32 GTA2_DIK_SPACE        = 0x39;
constexpr u32 GTA2_DIK_CAPITAL      = 0x3A;
constexpr u32 GTA2_DIK_F1           = 0x3B;
constexpr u32 GTA2_DIK_F2           = 0x3C;
constexpr u32 GTA2_DIK_F3           = 0x3D;
constexpr u32 GTA2_DIK_F4           = 0x3E;
constexpr u32 GTA2_DIK_F5           = 0x3F;
constexpr u32 GTA2_DIK_F6           = 0x40;
constexpr u32 GTA2_DIK_F7           = 0x41;
constexpr u32 GTA2_DIK_F8           = 0x42;
constexpr u32 GTA2_DIK_F9           = 0x43;
constexpr u32 GTA2_DIK_F10          = 0x44;
constexpr u32 GTA2_DIK_NUMLOCK      = 0x45;
constexpr u32 GTA2_DIK_SCROLL       = 0x46;
constexpr u32 GTA2_DIK_NUMPAD7      = 0x47;
constexpr u32 GTA2_DIK_NUMPAD8      = 0x48;
constexpr u32 GTA2_DIK_NUMPAD9      = 0x49;
constexpr u32 GTA2_DIK_NUMPADMINUS  = 0x4A;
constexpr u32 GTA2_DIK_NUMPAD4      = 0x4B;
constexpr u32 GTA2_DIK_NUMPAD5      = 0x4C;
constexpr u32 GTA2_DIK_NUMPAD6      = 0x4D;
constexpr u32 GTA2_DIK_NUMPADPLUS   = 0x4E;
constexpr u32 GTA2_DIK_NUMPAD1      = 0x4F;
constexpr u32 GTA2_DIK_NUMPAD2      = 0x50;
constexpr u32 GTA2_DIK_NUMPAD3      = 0x51;
constexpr u32 GTA2_DIK_NUMPAD0      = 0x52;
constexpr u32 GTA2_DIK_NUMPADPERIOD = 0x53;
constexpr u32 GTA2_DIK_F11          = 0x57;
constexpr u32 GTA2_DIK_F12          = 0x58;
constexpr u32 GTA2_DIK_NUMPADENTER  = 0x9C;
constexpr u32 GTA2_DIK_RCONTROL     = 0x9D;
constexpr u32 GTA2_DIK_NUMPADSLASH  = 0xB5;
constexpr u32 GTA2_DIK_RALT         = 0xB8;
constexpr u32 GTA2_DIK_UP           = 0xC8;
constexpr u32 GTA2_DIK_DOWN         = 0xD0;
constexpr u32 GTA2_DIK_LEFT         = 0xCB;
constexpr u32 GTA2_DIK_RIGHT        = 0xCD;
constexpr u32 GTA2_DIK_INSERT       = 0xD2;
constexpr u32 GTA2_DIK_DELETE       = 0xD3;
constexpr u32 GTA2_DIK_HOME         = 0xC7;
constexpr u32 GTA2_DIK_END          = 0xCF;
constexpr u32 GTA2_DIK_PRIOR        = 0xC9;
constexpr u32 GTA2_DIK_NEXT         = 0xD1;

u32 SdlScancodeToDik(SDL_Scancode scancode);
SDL_Scancode DikToSdlScancode(u32 dik);

} // namespace gta2::hal