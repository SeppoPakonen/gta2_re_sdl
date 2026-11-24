#pragma once
#include "HalTypes.hpp"

namespace gta2::hal {

class IRenderer;
class IAudioBackend;

class IMoviePlayer {
public:
    virtual ~IMoviePlayer() = default;

    virtual bool Open(const char* moviePath, IRenderer* pRenderer, IAudioBackend* pAudio) = 0;
    virtual void Close() = 0;

    virtual bool TickFrame() = 0; // Returns false if playback finished
    virtual void RenderFrame() = 0;
    virtual bool IsFinished() const = 0;
};

} // namespace gta2::hal