#include "FFmpegMoviePlayer.hpp"
#include "HalContext.hpp"

#include <cstdio>
#include <cstring>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

namespace gta2::hal {

static void LogAvError(const char* what, int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(err, buf, sizeof(buf));
    GTA2_VLOG("[HAL] FFmpegMoviePlayer: %s failed: %s\n", what, buf);
}

FFmpegMoviePlayer::~FFmpegMoviePlayer() {
    Close();
}

bool FFmpegMoviePlayer::Open(const char* moviePath, IRenderer* pRenderer, IAudioBackend* pAudio) {
    Close();
    if (!moviePath || !pRenderer) {
        return false;
    }

    mRenderer = pRenderer;
    mAudio = pAudio;

    // The game passes a virtual Windows-style path (e.g. "Movie\\intro.bik");
    // resolve it through the VFS so the real asset (including the wine install)
    // is found regardless of the current working directory.
    std::string resolved;
    if (auto pFs = GetHal().pFileSystem) {
        resolved = pFs->ResolvePath(moviePath);
    }
    const char* realPath = resolved.empty() ? moviePath : resolved.c_str();

    FILE* probe = fopen(realPath, "rb");
    if (!probe) {
        GTA2_VLOG("[HAL] FFmpegMoviePlayer: '%s' (resolved '%s') not found, skipping\n", moviePath, realPath);
        mFinished = true;
        return false;
    }
    fclose(probe);

    int rc = avformat_open_input(&mFmt, realPath, nullptr, nullptr);
    if (rc < 0) {
        LogAvError("avformat_open_input", rc);
        mFinished = true;
        return false;
    }
    rc = avformat_find_stream_info(mFmt, nullptr);
    if (rc < 0) {
        LogAvError("avformat_find_stream_info", rc);
        Close();
        return false;
    }

    const AVCodec* vDec = nullptr;
    mVideoStream = av_find_best_stream(mFmt, AVMEDIA_TYPE_VIDEO, -1, -1, &vDec, 0);
    if (mVideoStream < 0 || !vDec) {
        GTA2_VLOG("[HAL] FFmpegMoviePlayer: no video stream in '%s'\n", moviePath);
        Close();
        return false;
    }

    mVideoCtx = avcodec_alloc_context3(vDec);
    if (!mVideoCtx) {
        Close();
        return false;
    }
    rc = avcodec_parameters_to_context(mVideoCtx, mFmt->streams[mVideoStream]->codecpar);
    if (rc < 0) {
        LogAvError("avcodec_parameters_to_context(video)", rc);
        Close();
        return false;
    }
    rc = avcodec_open2(mVideoCtx, vDec, nullptr);
    if (rc < 0) {
        LogAvError("avcodec_open2(video)", rc);
        Close();
        return false;
    }

    const AVCodec* aDec = nullptr;
    mAudioStream = av_find_best_stream(mFmt, AVMEDIA_TYPE_AUDIO, -1, -1, &aDec, 0);
    if (mAudioStream >= 0 && aDec) {
        mAudioCtx = avcodec_alloc_context3(aDec);
        if (mAudioCtx) {
            rc = avcodec_parameters_to_context(mAudioCtx, mFmt->streams[mAudioStream]->codecpar);
            if (rc == 0) {
                rc = avcodec_open2(mAudioCtx, aDec, nullptr);
                if (rc < 0) {
                    LogAvError("avcodec_open2(audio)", rc);
                    avcodec_free_context(&mAudioCtx);
                    mAudioCtx = nullptr;
                    mAudioStream = -1;
                }
            } else {
                LogAvError("avcodec_parameters_to_context(audio)", rc);
                avcodec_free_context(&mAudioCtx);
                mAudioCtx = nullptr;
                mAudioStream = -1;
            }
        }
    }

    mFrame = av_frame_alloc();
    mRgb = av_frame_alloc();
    mPkt = av_packet_alloc();
    if (!mFrame || !mRgb || !mPkt) {
        Close();
        return false;
    }

    mWidth = mVideoCtx->width > 0 ? mVideoCtx->width : 640;
    mHeight = mVideoCtx->height > 0 ? mVideoCtx->height : 480;
    mFrameBuffer.assign(static_cast<size_t>(mWidth) * mHeight, 0xFF000000u);

    TextureDesc desc{};
    desc.width = static_cast<u16>(mWidth);
    desc.height = static_cast<u16>(mHeight);
    desc.bitsPerPixel = 32;
    desc.pPixels = mFrameBuffer.data();
    desc.pPalette = nullptr;
    desc.bTransparent = false;
    mTexture = mRenderer->RegisterTexture(desc);

    // Decode the whole audio track up front (intro audio is small) and start it;
    // video is streamed frame-by-frame from the beginning afterwards.
    DecodeAudioAll();

    mFinished = false;
    GTA2_VLOG("[HAL] FFmpegMoviePlayer: opened '%s' (%dx%d)\n", moviePath, mWidth, mHeight);
    return true;
}

void FFmpegMoviePlayer::Close() {
    if (mChannel != kInvalidChannelHandle && mAudio) {
        mAudio->StopChannel(mChannel);
        mChannel = kInvalidChannelHandle;
    }
    if (mSound != kInvalidSoundId && mAudio) {
        mAudio->UnloadSample(mSound);
        mSound = kInvalidSoundId;
    }
    mAudioPcm.clear();

    if (mTexture != kInvalidTextureHandle && mRenderer) {
        mRenderer->UnregisterTexture(mTexture);
        mTexture = kInvalidTextureHandle;
    }

    if (mSws)      { sws_freeContext(mSws); mSws = nullptr; }
    if (mSwr)      { swr_free(&mSwr); mSwr = nullptr; }
    if (mFrame)    { av_frame_free(&mFrame); mFrame = nullptr; }
    if (mRgb)      { av_frame_free(&mRgb); mRgb = nullptr; }
    if (mPkt)      { av_packet_free(&mPkt); mPkt = nullptr; }
    if (mVideoCtx) { avcodec_free_context(&mVideoCtx); mVideoCtx = nullptr; }
    if (mAudioCtx) { avcodec_free_context(&mAudioCtx); mAudioCtx = nullptr; }
    if (mFmt)      { avformat_close_input(&mFmt); mFmt = nullptr; }

    mVideoStream = -1;
    mAudioStream = -1;
    mWidth = 0;
    mHeight = 0;
    mFrameBuffer.clear();
    mAudioSampleRate = 0;
    mAudioChannels = 0;
    mRenderer = nullptr;
    mAudio = nullptr;
    mFinished = true;
}

bool FFmpegMoviePlayer::DecodeAudioAll() {
    if (!mAudioCtx || mAudioStream < 0 || !mAudio) {
        return true;
    }

    AVChannelLayout inLayout = mAudioCtx->ch_layout;
    mAudioChannels = inLayout.nb_channels > 0 ? inLayout.nb_channels : 2;
    mAudioSampleRate = mAudioCtx->sample_rate > 0 ? mAudioCtx->sample_rate : 44100;

    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, mAudioChannels);

    int swrRc = swr_alloc_set_opts2(&mSwr, &outLayout, AV_SAMPLE_FMT_S16, mAudioSampleRate,
                                    &inLayout, mAudioCtx->sample_fmt, mAudioSampleRate, 0, nullptr);
    if (swrRc < 0 || !mSwr) {
        LogAvError("swr_alloc_set_opts2", swrRc);
        return false;
    }
    if (swr_init(mSwr) < 0) {
        GTA2_VLOG("[HAL] FFmpegMoviePlayer: swr_init failed\n");
        return false;
    }
    av_channel_layout_uninit(&outLayout);

    AVFrame* decoded = av_frame_alloc();
    if (!decoded) {
        return false;
    }

    int outChStride = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    std::vector<u8> outBuf;
    outBuf.resize(static_cast<size_t>(outChStride) * mAudioChannels * 1024);

    while (av_read_frame(mFmt, mPkt) >= 0) {
        if (mPkt->stream_index == mAudioStream) {
            if (avcodec_send_packet(mAudioCtx, mPkt) == 0) {
                while (avcodec_receive_frame(mAudioCtx, decoded) == 0) {
                    int maxOut = av_rescale_rnd(swr_get_delay(mSwr, mAudioSampleRate) + decoded->nb_samples,
                                                mAudioSampleRate, mAudioSampleRate, AV_ROUND_UP);
                    if (maxOut <= 0) {
                        maxOut = decoded->nb_samples + 1024;
                    }
                    size_t need = static_cast<size_t>(outChStride) * mAudioChannels * maxOut;
                    if (outBuf.size() < need) {
                        outBuf.resize(need);
                    }
                    uint8_t* dst[1] = { outBuf.data() };
                    int outSamples = swr_convert(mSwr, dst, maxOut,
                                                 (const uint8_t**)decoded->extended_data, decoded->nb_samples);
                    if (outSamples > 0) {
                        s16* s = reinterpret_cast<s16*>(outBuf.data());
                        mAudioPcm.insert(mAudioPcm.end(), s, s + static_cast<size_t>(outSamples) * mAudioChannels);
                    }
                }
            }
        }
        av_packet_unref(mPkt);
    }

    // Flush any buffered samples.
    int maxOut = av_rescale_rnd(swr_get_delay(mSwr, mAudioSampleRate) + 1024,
                                mAudioSampleRate, mAudioSampleRate, AV_ROUND_UP);
    if (maxOut > 0) {
        if (outBuf.size() < static_cast<size_t>(outChStride) * mAudioChannels * maxOut) {
            outBuf.resize(static_cast<size_t>(outChStride) * mAudioChannels * maxOut);
        }
        uint8_t* dst[1] = { outBuf.data() };
        int outSamples = swr_convert(mSwr, dst, maxOut, nullptr, 0);
        if (outSamples > 0) {
            s16* s = reinterpret_cast<s16*>(outBuf.data());
            mAudioPcm.insert(mAudioPcm.end(), s, s + static_cast<size_t>(outSamples) * mAudioChannels);
        }
    }

    av_frame_free(&decoded);

    if (!mAudioPcm.empty()) {
        mSound = mAudio->LoadSampleFromMemory(mAudioPcm.data(),
                                              mAudioPcm.size() * sizeof(s16),
                                              mAudioSampleRate, 16, mAudioChannels);
        if (mSound != kInvalidSoundId) {
            mChannel = mAudio->PlaySample(mSound, 1.0f, 0.0f, false);
        }
    }

    // Rewind for video streaming.
    avformat_seek_file(mFmt, mVideoStream, INT64_MIN, INT64_MIN, INT64_MIN, AVSEEK_FLAG_FRAME);
    avcodec_flush_buffers(mVideoCtx);
    return true;
}

bool FFmpegMoviePlayer::DecodeNextVideoFrame() {
    while (true) {
        int rc = av_read_frame(mFmt, mPkt);
        if (rc < 0) {
            return false; // EOF
        }
        if (mPkt->stream_index != mVideoStream) {
            av_packet_unref(mPkt);
            continue;
        }

        avcodec_send_packet(mVideoCtx, mPkt);
        av_packet_unref(mPkt);

        while (avcodec_receive_frame(mVideoCtx, mFrame) == 0) {
            if (!mSws) {
                mSws = sws_getContext(mWidth, mHeight,
                                      static_cast<AVPixelFormat>(mFrame->format),
                                      mWidth, mHeight, AV_PIX_FMT_RGBA,
                                      SWS_BILINEAR, nullptr, nullptr, nullptr);
                if (!mSws) {
                    GTA2_VLOG("[HAL] FFmpegMoviePlayer: sws_getContext failed\n");
                    return false;
                }
            }

            mRgb->format = AV_PIX_FMT_RGBA;
            mRgb->width = mWidth;
            mRgb->height = mHeight;
            av_frame_get_buffer(mRgb, 1);
            sws_scale(mSws, mFrame->data, mFrame->linesize, 0, mHeight,
                      mRgb->data, mRgb->linesize);

            const u32* src = reinterpret_cast<const u32*>(mRgb->data[0]);
            size_t count = static_cast<size_t>(mWidth) * mHeight;
            memcpy(mFrameBuffer.data(), src, count * sizeof(u32));

            if (mTexture != kInvalidTextureHandle) {
                mRenderer->UpdateTexture(mTexture, mFrameBuffer.data(), nullptr, nullptr,
                                         static_cast<u16>(mWidth), static_cast<u16>(mHeight));
            }
            av_frame_unref(mRgb);
            return true;
        }
    }
}

bool FFmpegMoviePlayer::TickFrame() {
    if (mFinished) {
        return false;
    }
    if (!DecodeNextVideoFrame()) {
        mFinished = true;
        if (mChannel != kInvalidChannelHandle && mAudio) {
            mAudio->StopChannel(mChannel);
            mChannel = kInvalidChannelHandle;
        }
        return false;
    }
    return true;
}

void FFmpegMoviePlayer::RenderFrame() {
    if (mFinished || !mRenderer || mTexture == kInvalidTextureHandle) {
        return;
    }

    mRenderer->BeginScene();

    HalVertex verts[4];
    const f32 w = 640.0f;
    const f32 h = 480.0f;

    verts[0] = { 0.0f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFFu, 0, 0.0f, 0.0f };
    verts[1] = { w,    0.0f, 0.0f, 1.0f, 0xFFFFFFFFu, 0, 1.0f, 0.0f };
    verts[2] = { w,    h,    0.0f, 1.0f, 0xFFFFFFFFu, 0, 1.0f, 1.0f };
    verts[3] = { 0.0f, h,    0.0f, 1.0f, 0xFFFFFFFFu, 0, 0.0f, 1.0f };

    mRenderer->DrawQuad(verts, mTexture, RENDER_NONE, 0xFFFFFFFFu);
    mRenderer->EndScene();
}

} // namespace gta2::hal