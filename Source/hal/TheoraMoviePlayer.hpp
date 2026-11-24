#pragma once

#include "IMoviePlayer.hpp"
#include "IRenderer.hpp"
#include "IAudioBackend.hpp"
#include <string>
#include <vector>

namespace gta2::hal {

class TheoraMoviePlayer : public IMoviePlayer {
private:
    IRenderer*          mRenderer = nullptr;
    IAudioBackend*      mAudio    = nullptr;
    TextureHandle       mTexture  = kInvalidTextureHandle;
    bool                mFinished = true;
    s32                 mWidth    = 0;
    s32                 mHeight   = 0;
    std::string         mMoviePath;
    std::vector<u32>    mFrameBuffer;
    u32                 mCurrentFrame = 0;
    u32                 mTotalFrames  = 0;

public:
    TheoraMoviePlayer() = default;
    ~TheoraMoviePlayer() override;

    bool Open(const char* moviePath, IRenderer* pRenderer, IAudioBackend* pAudio) override;
    void Close() override;
    bool TickFrame() override;
    void RenderFrame() override;
    bool IsFinished() const override { return mFinished; }
};

} // namespace gta2::hal
