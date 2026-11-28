#pragma once

#include "IMoviePlayer.hpp"
#include "IRenderer.hpp"
#include "IAudioBackend.hpp"
#include "types.hpp"

#include <string>
#include <vector>
#include <chrono>

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;

namespace gta2::hal {

// Movie player backed by libav* (ffmpeg). libavformat auto-detects the
// container/codec, so the original Bink intro (intro.bik / preintro.bik) as
// well as re-encoded mp4/mkv files play through the same path.
class FFmpegMoviePlayer : public IMoviePlayer {
public:
    FFmpegMoviePlayer() = default;
    ~FFmpegMoviePlayer() override;

    bool Open(const char* moviePath, IRenderer* pRenderer, IAudioBackend* pAudio) override;
    void Close() override;
    bool TickFrame() override;
    void RenderFrame() override;
    bool IsFinished() const override { return mFinished; }

private:
    bool DecodeAudioAll();
    bool DecodeNextVideoFrame();

    IRenderer*     mRenderer = nullptr;
    IAudioBackend* mAudio    = nullptr;
    TextureHandle  mTexture  = kInvalidTextureHandle;
    bool           mFinished = true;

    AVFormatContext* mFmt      = nullptr;
    AVCodecContext*  mVideoCtx = nullptr;
    AVCodecContext*  mAudioCtx = nullptr;
    SwsContext*      mSws      = nullptr;
    SwrContext*      mSwr      = nullptr;
    AVFrame*         mFrame    = nullptr;
    AVFrame*         mRgb      = nullptr;
    AVPacket*        mPkt      = nullptr;
    int              mVideoStream = -1;
    int              mAudioStream = -1;

    s32 mWidth  = 0;
    s32 mHeight = 0;
    std::string mResolvedPath;
    std::vector<u32> mFrameBuffer; // RGBA8888, mWidth*mHeight

    u32          mFrameCount = 0;
    double       mFrameInterval = 0.0; // seconds per frame (for pacing)
    std::chrono::steady_clock::time_point mNextFrameClock;
    bool         mClockInit = false;
    SoundId      mSound  = kInvalidSoundId;
    ChannelHandle mChannel = kInvalidChannelHandle;
    std::vector<s16> mAudioPcm; // interleaved s16
    s32          mAudioSampleRate = 0;
    s32          mAudioChannels    = 0;
};

} // namespace gta2::hal