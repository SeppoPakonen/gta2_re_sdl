#include "InputKeyMap.hpp"

namespace gta2::hal {

u32 SdlScancodeToDik(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_A:                return GTA2_DIK_A;
        case SDL_SCANCODE_B:                return GTA2_DIK_B;
        case SDL_SCANCODE_C:                return GTA2_DIK_C;
        case SDL_SCANCODE_D:                return GTA2_DIK_D;
        case SDL_SCANCODE_E:                return GTA2_DIK_E;
        case SDL_SCANCODE_F:                return GTA2_DIK_F;
        case SDL_SCANCODE_G:                return GTA2_DIK_G;
        case SDL_SCANCODE_H:                return GTA2_DIK_H;
        case SDL_SCANCODE_I:                return GTA2_DIK_I;
        case SDL_SCANCODE_J:                return GTA2_DIK_J;
        case SDL_SCANCODE_K:                return GTA2_DIK_K;
        case SDL_SCANCODE_L:                return GTA2_DIK_L;
        case SDL_SCANCODE_M:                return GTA2_DIK_M;
        case SDL_SCANCODE_N:                return GTA2_DIK_N;
        case SDL_SCANCODE_O:                return GTA2_DIK_O;
        case SDL_SCANCODE_P:                return GTA2_DIK_P;
        case SDL_SCANCODE_Q:                return GTA2_DIK_Q;
        case SDL_SCANCODE_R:                return GTA2_DIK_R;
        case SDL_SCANCODE_S:                return GTA2_DIK_S;
        case SDL_SCANCODE_T:                return GTA2_DIK_T;
        case SDL_SCANCODE_U:                return GTA2_DIK_U;
        case SDL_SCANCODE_V:                return GTA2_DIK_V;
        case SDL_SCANCODE_W:                return GTA2_DIK_W;
        case SDL_SCANCODE_X:                return GTA2_DIK_X;
        case SDL_SCANCODE_Y:                return GTA2_DIK_Y;
        case SDL_SCANCODE_Z:                return GTA2_DIK_Z;

        case SDL_SCANCODE_1:                return GTA2_DIK_1;
        case SDL_SCANCODE_2:                return GTA2_DIK_2;
        case SDL_SCANCODE_3:                return GTA2_DIK_3;
        case SDL_SCANCODE_4:                return GTA2_DIK_4;
        case SDL_SCANCODE_5:                return GTA2_DIK_5;
        case SDL_SCANCODE_6:                return GTA2_DIK_6;
        case SDL_SCANCODE_7:                return GTA2_DIK_7;
        case SDL_SCANCODE_8:                return GTA2_DIK_8;
        case SDL_SCANCODE_9:                return GTA2_DIK_9;
        case SDL_SCANCODE_0:                return GTA2_DIK_0;

        case SDL_SCANCODE_RETURN:           return GTA2_DIK_RETURN;
        case SDL_SCANCODE_ESCAPE:           return GTA2_DIK_ESCAPE;
        case SDL_SCANCODE_BACKSPACE:        return GTA2_DIK_BACKSPACE;
        case SDL_SCANCODE_TAB:              return GTA2_DIK_TAB;
        case SDL_SCANCODE_SPACE:            return GTA2_DIK_SPACE;
        case SDL_SCANCODE_MINUS:            return GTA2_DIK_MINUS;
        case SDL_SCANCODE_EQUALS:           return GTA2_DIK_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET:      return GTA2_DIK_LBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET:     return GTA2_DIK_RBRACKET;
        case SDL_SCANCODE_BACKSLASH:        return GTA2_DIK_BACKSLASH;
        case SDL_SCANCODE_SEMICOLON:        return GTA2_DIK_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE:       return GTA2_DIK_APOSTROPHE;
        case SDL_SCANCODE_GRAVE:            return GTA2_DIK_GRAVE;
        case SDL_SCANCODE_COMMA:            return GTA2_DIK_COMMA;
        case SDL_SCANCODE_PERIOD:           return GTA2_DIK_PERIOD;
        case SDL_SCANCODE_SLASH:            return GTA2_DIK_SLASH;
        case SDL_SCANCODE_CAPSLOCK:         return GTA2_DIK_CAPITAL;

        case SDL_SCANCODE_F1:               return GTA2_DIK_F1;
        case SDL_SCANCODE_F2:               return GTA2_DIK_F2;
        case SDL_SCANCODE_F3:               return GTA2_DIK_F3;
        case SDL_SCANCODE_F4:               return GTA2_DIK_F4;
        case SDL_SCANCODE_F5:               return GTA2_DIK_F5;
        case SDL_SCANCODE_F6:               return GTA2_DIK_F6;
        case SDL_SCANCODE_F7:               return GTA2_DIK_F7;
        case SDL_SCANCODE_F8:               return GTA2_DIK_F8;
        case SDL_SCANCODE_F9:               return GTA2_DIK_F9;
        case SDL_SCANCODE_F10:              return GTA2_DIK_F10;
        case SDL_SCANCODE_F11:              return GTA2_DIK_F11;
        case SDL_SCANCODE_F12:              return GTA2_DIK_F12;

        case SDL_SCANCODE_SCROLLLOCK:       return GTA2_DIK_SCROLL;

        case SDL_SCANCODE_INSERT:           return GTA2_DIK_INSERT;
        case SDL_SCANCODE_HOME:             return GTA2_DIK_HOME;
        case SDL_SCANCODE_PAGEUP:           return GTA2_DIK_PRIOR;
        case SDL_SCANCODE_DELETE:           return GTA2_DIK_DELETE;
        case SDL_SCANCODE_END:              return GTA2_DIK_END;
        case SDL_SCANCODE_PAGEDOWN:         return GTA2_DIK_NEXT;
        case SDL_SCANCODE_RIGHT:            return GTA2_DIK_RIGHT;
        case SDL_SCANCODE_LEFT:             return GTA2_DIK_LEFT;
        case SDL_SCANCODE_DOWN:             return GTA2_DIK_DOWN;
        case SDL_SCANCODE_UP:               return GTA2_DIK_UP;

        case SDL_SCANCODE_NUMLOCKCLEAR:     return GTA2_DIK_NUMLOCK;
        case SDL_SCANCODE_KP_DIVIDE:        return GTA2_DIK_NUMPADSLASH;
        case SDL_SCANCODE_KP_MULTIPLY:      return GTA2_DIK_NUMPADSTAR;
        case SDL_SCANCODE_KP_MINUS:         return GTA2_DIK_NUMPADMINUS;
        case SDL_SCANCODE_KP_PLUS:          return GTA2_DIK_NUMPADPLUS;
        case SDL_SCANCODE_KP_ENTER:         return GTA2_DIK_NUMPADENTER;
        case SDL_SCANCODE_KP_1:             return GTA2_DIK_NUMPAD1;
        case SDL_SCANCODE_KP_2:             return GTA2_DIK_NUMPAD2;
        case SDL_SCANCODE_KP_3:             return GTA2_DIK_NUMPAD3;
        case SDL_SCANCODE_KP_4:             return GTA2_DIK_NUMPAD4;
        case SDL_SCANCODE_KP_5:             return GTA2_DIK_NUMPAD5;
        case SDL_SCANCODE_KP_6:             return GTA2_DIK_NUMPAD6;
        case SDL_SCANCODE_KP_7:             return GTA2_DIK_NUMPAD7;
        case SDL_SCANCODE_KP_8:             return GTA2_DIK_NUMPAD8;
        case SDL_SCANCODE_KP_9:             return GTA2_DIK_NUMPAD9;
        case SDL_SCANCODE_KP_0:             return GTA2_DIK_NUMPAD0;
        case SDL_SCANCODE_KP_PERIOD:        return GTA2_DIK_NUMPADPERIOD;

        case SDL_SCANCODE_LCTRL:            return GTA2_DIK_LCONTROL;
        case SDL_SCANCODE_LSHIFT:           return GTA2_DIK_LSHIFT;
        case SDL_SCANCODE_LALT:             return GTA2_DIK_LALT;
        case SDL_SCANCODE_RCTRL:            return GTA2_DIK_RCONTROL;
        case SDL_SCANCODE_RSHIFT:           return GTA2_DIK_RSHIFT;
        case SDL_SCANCODE_RALT:             return GTA2_DIK_RALT;

        default:                            return 0;
    }
}

SDL_Scancode DikToSdlScancode(u32 dik) {
    switch (dik) {
        case GTA2_DIK_A:                return SDL_SCANCODE_A;
        case GTA2_DIK_B:                return SDL_SCANCODE_B;
        case GTA2_DIK_C:                return SDL_SCANCODE_C;
        case GTA2_DIK_D:                return SDL_SCANCODE_D;
        case GTA2_DIK_E:                return SDL_SCANCODE_E;
        case GTA2_DIK_F:                return SDL_SCANCODE_F;
        case GTA2_DIK_G:                return SDL_SCANCODE_G;
        case GTA2_DIK_H:                return SDL_SCANCODE_H;
        case GTA2_DIK_I:                return SDL_SCANCODE_I;
        case GTA2_DIK_J:                return SDL_SCANCODE_J;
        case GTA2_DIK_K:                return SDL_SCANCODE_K;
        case GTA2_DIK_L:                return SDL_SCANCODE_L;
        case GTA2_DIK_M:                return SDL_SCANCODE_M;
        case GTA2_DIK_N:                return SDL_SCANCODE_N;
        case GTA2_DIK_O:                return SDL_SCANCODE_O;
        case GTA2_DIK_P:                return SDL_SCANCODE_P;
        case GTA2_DIK_Q:                return SDL_SCANCODE_Q;
        case GTA2_DIK_R:                return SDL_SCANCODE_R;
        case GTA2_DIK_S:                return SDL_SCANCODE_S;
        case GTA2_DIK_T:                return SDL_SCANCODE_T;
        case GTA2_DIK_U:                return SDL_SCANCODE_U;
        case GTA2_DIK_V:                return SDL_SCANCODE_V;
        case GTA2_DIK_W:                return SDL_SCANCODE_W;
        case GTA2_DIK_X:                return SDL_SCANCODE_X;
        case GTA2_DIK_Y:                return SDL_SCANCODE_Y;
        case GTA2_DIK_Z:                return SDL_SCANCODE_Z;

        case GTA2_DIK_1:                return SDL_SCANCODE_1;
        case GTA2_DIK_2:                return SDL_SCANCODE_2;
        case GTA2_DIK_3:                return SDL_SCANCODE_3;
        case GTA2_DIK_4:                return SDL_SCANCODE_4;
        case GTA2_DIK_5:                return SDL_SCANCODE_5;
        case GTA2_DIK_6:                return SDL_SCANCODE_6;
        case GTA2_DIK_7:                return SDL_SCANCODE_7;
        case GTA2_DIK_8:                return SDL_SCANCODE_8;
        case GTA2_DIK_9:                return SDL_SCANCODE_9;
        case GTA2_DIK_0:                return SDL_SCANCODE_0;

        case GTA2_DIK_MINUS:            return SDL_SCANCODE_MINUS;
        case GTA2_DIK_EQUALS:           return SDL_SCANCODE_EQUALS;
        case GTA2_DIK_BACKSPACE:        return SDL_SCANCODE_BACKSPACE;
        case GTA2_DIK_TAB:              return SDL_SCANCODE_TAB;
        case GTA2_DIK_LBRACKET:         return SDL_SCANCODE_LEFTBRACKET;
        case GTA2_DIK_RBRACKET:         return SDL_SCANCODE_RIGHTBRACKET;
        case GTA2_DIK_BACKSLASH:        return SDL_SCANCODE_BACKSLASH;
        case GTA2_DIK_SEMICOLON:        return SDL_SCANCODE_SEMICOLON;
        case GTA2_DIK_APOSTROPHE:       return SDL_SCANCODE_APOSTROPHE;
        case GTA2_DIK_GRAVE:            return SDL_SCANCODE_GRAVE;
        case GTA2_DIK_COMMA:            return SDL_SCANCODE_COMMA;
        case GTA2_DIK_PERIOD:           return SDL_SCANCODE_PERIOD;
        case GTA2_DIK_SLASH:            return SDL_SCANCODE_SLASH;
        case GTA2_DIK_CAPITAL:          return SDL_SCANCODE_CAPSLOCK;

        case GTA2_DIK_F1:               return SDL_SCANCODE_F1;
        case GTA2_DIK_F2:               return SDL_SCANCODE_F2;
        case GTA2_DIK_F3:               return SDL_SCANCODE_F3;
        case GTA2_DIK_F4:               return SDL_SCANCODE_F4;
        case GTA2_DIK_F5:               return SDL_SCANCODE_F5;
        case GTA2_DIK_F6:               return SDL_SCANCODE_F6;
        case GTA2_DIK_F7:               return SDL_SCANCODE_F7;
        case GTA2_DIK_F8:               return SDL_SCANCODE_F8;
        case GTA2_DIK_F9:               return SDL_SCANCODE_F9;
        case GTA2_DIK_F10:              return SDL_SCANCODE_F10;
        case GTA2_DIK_F11:              return SDL_SCANCODE_F11;
        case GTA2_DIK_F12:              return SDL_SCANCODE_F12;

        case GTA2_DIK_SCROLL:           return SDL_SCANCODE_SCROLLLOCK;

        case GTA2_DIK_INSERT:           return SDL_SCANCODE_INSERT;
        case GTA2_DIK_HOME:             return SDL_SCANCODE_HOME;
        case GTA2_DIK_PRIOR:            return SDL_SCANCODE_PAGEUP;
        case GTA2_DIK_DELETE:           return SDL_SCANCODE_DELETE;
        case GTA2_DIK_END:              return SDL_SCANCODE_END;
        case GTA2_DIK_NEXT:             return SDL_SCANCODE_PAGEDOWN;
        case GTA2_DIK_RIGHT:            return SDL_SCANCODE_RIGHT;
        case GTA2_DIK_LEFT:             return SDL_SCANCODE_LEFT;
        case GTA2_DIK_DOWN:             return SDL_SCANCODE_DOWN;
        case GTA2_DIK_UP:               return SDL_SCANCODE_UP;

        case GTA2_DIK_NUMLOCK:          return SDL_SCANCODE_NUMLOCKCLEAR;
        case GTA2_DIK_NUMPADSLASH:      return SDL_SCANCODE_KP_DIVIDE;
        case GTA2_DIK_NUMPADSTAR:       return SDL_SCANCODE_KP_MULTIPLY;
        case GTA2_DIK_NUMPADMINUS:      return SDL_SCANCODE_KP_MINUS;
        case GTA2_DIK_NUMPADPLUS:       return SDL_SCANCODE_KP_PLUS;
        case GTA2_DIK_NUMPADENTER:      return SDL_SCANCODE_KP_ENTER;
        case GTA2_DIK_NUMPAD1:          return SDL_SCANCODE_KP_1;
        case GTA2_DIK_NUMPAD2:          return SDL_SCANCODE_KP_2;
        case GTA2_DIK_NUMPAD3:          return SDL_SCANCODE_KP_3;
        case GTA2_DIK_NUMPAD4:          return SDL_SCANCODE_KP_4;
        case GTA2_DIK_NUMPAD5:          return SDL_SCANCODE_KP_5;
        case GTA2_DIK_NUMPAD6:          return SDL_SCANCODE_KP_6;
        case GTA2_DIK_NUMPAD7:          return SDL_SCANCODE_KP_7;
        case GTA2_DIK_NUMPAD8:          return SDL_SCANCODE_KP_8;
        case GTA2_DIK_NUMPAD9:          return SDL_SCANCODE_KP_9;
        case GTA2_DIK_NUMPAD0:          return SDL_SCANCODE_KP_0;
        case GTA2_DIK_NUMPADPERIOD:     return SDL_SCANCODE_KP_PERIOD;

        case GTA2_DIK_LCONTROL:         return SDL_SCANCODE_LCTRL;
        case GTA2_DIK_LSHIFT:           return SDL_SCANCODE_LSHIFT;
        case GTA2_DIK_LALT:             return SDL_SCANCODE_LALT;
        case GTA2_DIK_RCONTROL:         return SDL_SCANCODE_RCTRL;
        case GTA2_DIK_RSHIFT:           return SDL_SCANCODE_RSHIFT;
        case GTA2_DIK_RALT:             return SDL_SCANCODE_RALT;

        case GTA2_DIK_ESCAPE:           return SDL_SCANCODE_ESCAPE;
        case GTA2_DIK_RETURN:           return SDL_SCANCODE_RETURN;
        case GTA2_DIK_SPACE:            return SDL_SCANCODE_SPACE;

        default:                        return SDL_SCANCODE_UNKNOWN;
    }
}

} // namespace gta2::hal