#include "SDLEventSystem.hpp"

namespace gta2::hal {

#if defined(GTA2_USE_SDL3)

void SDLEventSystem::PumpEvents() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        Event ev{};
        switch (sdlEvent.type) {
            case SDL_EVENT_QUIT:
                mQuitRequested = true;
                ev.type = EventType::Quit;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_WINDOW_CLOSE:
                mQuitRequested = true;
                ev.type = EventType::WindowClose;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                ev.type = EventType::WindowFocusGained;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                ev.type = EventType::WindowFocusLost;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                ev.type = EventType::WindowResize;
                ev.windowResize.width = sdlEvent.window.data1;
                ev.windowResize.height = sdlEvent.window.data2;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_KEY_DOWN:
                ev.type = EventType::KeyDown;
                ev.key.scancode = static_cast<u32>(sdlEvent.key.scancode);
                ev.key.repeat = sdlEvent.key.repeat;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_KEY_UP:
                ev.type = EventType::KeyUp;
                ev.key.scancode = static_cast<u32>(sdlEvent.key.scancode);
                ev.key.repeat = false;
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                ev.type = EventType::GamepadButtonDown;
                ev.gamepadButton.button = sdlEvent.gbutton.button;
                ev.gamepadButton.gamepadIndex = static_cast<u8>(sdlEvent.gbutton.which);
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                ev.type = EventType::GamepadButtonUp;
                ev.gamepadButton.button = sdlEvent.gbutton.button;
                ev.gamepadButton.gamepadIndex = static_cast<u8>(sdlEvent.gbutton.which);
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                ev.type = EventType::GamepadAxisMotion;
                ev.gamepadAxis.axis = sdlEvent.gaxis.axis;
                ev.gamepadAxis.value = sdlEvent.gaxis.value / 32767.0f;
                ev.gamepadAxis.gamepadIndex = static_cast<u8>(sdlEvent.gaxis.which);
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_FINGER_DOWN:
                ev.type = EventType::TouchDown;
                ev.touch.x = sdlEvent.tfinger.x;
                ev.touch.y = sdlEvent.tfinger.y;
                ev.touch.touchId = static_cast<u32>(sdlEvent.tfinger.fingerId);
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_FINGER_UP:
                ev.type = EventType::TouchUp;
                ev.touch.x = sdlEvent.tfinger.x;
                ev.touch.y = sdlEvent.tfinger.y;
                ev.touch.touchId = static_cast<u32>(sdlEvent.tfinger.fingerId);
                mEventQueue.push(ev);
                break;
            case SDL_EVENT_FINGER_MOTION:
                ev.type = EventType::TouchMotion;
                ev.touch.x = sdlEvent.tfinger.x;
                ev.touch.y = sdlEvent.tfinger.y;
                ev.touch.touchId = static_cast<u32>(sdlEvent.tfinger.fingerId);
                mEventQueue.push(ev);
                break;
            default:
                break;
        }
    }
}

#else // SDL2

void SDLEventSystem::PumpEvents() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        Event ev{};
        switch (sdlEvent.type) {
            case SDL_QUIT:
                mQuitRequested = true;
                ev.type = EventType::Quit;
                mEventQueue.push(ev);
                break;
            case SDL_WINDOWEVENT:
                switch (sdlEvent.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        mQuitRequested = true;
                        ev.type = EventType::WindowClose;
                        mEventQueue.push(ev);
                        break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        ev.type = EventType::WindowFocusGained;
                        mEventQueue.push(ev);
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        ev.type = EventType::WindowFocusLost;
                        mEventQueue.push(ev);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        ev.type = EventType::WindowResize;
                        ev.windowResize.width = sdlEvent.window.data1;
                        ev.windowResize.height = sdlEvent.window.data2;
                        mEventQueue.push(ev);
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYDOWN:
                ev.type = EventType::KeyDown;
                ev.key.scancode = static_cast<u32>(sdlEvent.key.keysym.scancode);
                ev.key.repeat = sdlEvent.key.repeat != 0;
                mEventQueue.push(ev);
                break;
            case SDL_KEYUP:
                ev.type = EventType::KeyUp;
                ev.key.scancode = static_cast<u32>(sdlEvent.key.keysym.scancode);
                ev.key.repeat = false;
                mEventQueue.push(ev);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                ev.type = EventType::GamepadButtonDown;
                ev.gamepadButton.button = sdlEvent.cbutton.button;
                ev.gamepadButton.gamepadIndex = static_cast<u8>(sdlEvent.cbutton.which);
                mEventQueue.push(ev);
                break;
            case SDL_CONTROLLERBUTTONUP:
                ev.type = EventType::GamepadButtonUp;
                ev.gamepadButton.button = sdlEvent.cbutton.button;
                ev.gamepadButton.gamepadIndex = static_cast<u8>(sdlEvent.cbutton.which);
                mEventQueue.push(ev);
                break;
            case SDL_CONTROLLERAXISMOTION:
                ev.type = EventType::GamepadAxisMotion;
                ev.gamepadAxis.axis = sdlEvent.caxis.axis;
                ev.gamepadAxis.value = sdlEvent.caxis.value / 32767.0f;
                ev.gamepadAxis.gamepadIndex = static_cast<u8>(sdlEvent.caxis.which);
                mEventQueue.push(ev);
                break;
            case SDL_FINGERDOWN:
                ev.type = EventType::TouchDown;
                ev.touch.x = sdlEvent.tfinger.x;
                ev.touch.y = sdlEvent.tfinger.y;
                ev.touch.touchId = static_cast<u32>(sdlEvent.tfinger.fingerId);
                mEventQueue.push(ev);
                break;
            case SDL_FINGERUP:
                ev.type = EventType::TouchUp;
                ev.touch.x = sdlEvent.tfinger.x;
                ev.touch.y = sdlEvent.tfinger.y;
                ev.touch.touchId = static_cast<u32>(sdlEvent.tfinger.fingerId);
                mEventQueue.push(ev);
                break;
            case SDL_FINGERMOTION:
                ev.type = EventType::TouchMotion;
                ev.touch.x = sdlEvent.tfinger.x;
                ev.touch.y = sdlEvent.tfinger.y;
                ev.touch.touchId = static_cast<u32>(sdlEvent.tfinger.fingerId);
                mEventQueue.push(ev);
                break;
            default:
                break;
        }
    }
}

#endif

bool SDLEventSystem::PollEvent(Event& event) {
    if (mEventQueue.empty()) {
        PumpEvents();
    }
    if (mEventQueue.empty()) {
        return false;
    }
    event = mEventQueue.front();
    mEventQueue.pop();
    return true;
}

} // namespace gta2::hal