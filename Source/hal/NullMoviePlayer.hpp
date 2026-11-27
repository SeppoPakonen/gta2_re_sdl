#pragma once

#include "IMoviePlayer.hpp"
#include "types.hpp"
#include <cstdio>

namespace gta2::hal {

class NullMoviePlayer : public IMoviePlayer {
public:
    NullMoviePlayer() = default;
    ~NullMoviePlayer() override = default;

    bool Open(const char* moviePath, IRenderer* pRenderer, IAudioBackend* pAudio) override {
        (void)pRenderer;
        (void)pAudio;
        GTA2_VLOG("[HAL] NullMoviePlayer: Skipping movie '%s'\n", moviePath ? moviePath : "null");
        return true;
    }

    void Close() override {}

    bool TickFrame() override {
        return false; // Immediately finished
    }

    void RenderFrame() override {}

    bool IsFinished() const override {
        return true;
    }
};

} // namespace gta2::hal
