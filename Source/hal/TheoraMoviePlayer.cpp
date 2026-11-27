#include "TheoraMoviePlayer.hpp"
#include "types.hpp"
#include <cstdio>
#include <cstring>

namespace gta2::hal {

TheoraMoviePlayer::~TheoraMoviePlayer() {
    Close();
}

bool TheoraMoviePlayer::Open(const char* moviePath, IRenderer* pRenderer, IAudioBackend* pAudio) {
    Close();

    if (!moviePath || !pRenderer) {
        return false;
    }

    mRenderer = pRenderer;
    mAudio = pAudio;
    mMoviePath = moviePath;

    // Check if file exists
    FILE* fp = fopen(moviePath, "rb");
    if (!fp) {
        GTA2_VLOG("[HAL] TheoraMoviePlayer: Movie '%s' not found, skipping\n", moviePath);
        mFinished = true;
        return false;
    }
    fclose(fp);

    GTA2_VLOG("[HAL] TheoraMoviePlayer: Opened movie '%s'\n", moviePath);
    mWidth = 640;
    mHeight = 480;
    mCurrentFrame = 0;
    mTotalFrames = 300; // Placeholder frame count
    mFinished = false;

    mFrameBuffer.resize(static_cast<size_t>(mWidth * mHeight), 0xFF000000);

    TextureDesc desc{};
    desc.width = static_cast<u16>(mWidth);
    desc.height = static_cast<u16>(mHeight);
    desc.bitsPerPixel = 32;
    desc.pPixels = mFrameBuffer.data();
    desc.pPalette = nullptr;
    desc.bTransparent = false;

    mTexture = mRenderer->RegisterTexture(desc);

    return true;
}

void TheoraMoviePlayer::Close() {
    if (mTexture != kInvalidTextureHandle && mRenderer) {
        mRenderer->UnregisterTexture(mTexture);
        mTexture = kInvalidTextureHandle;
    }
    mRenderer = nullptr;
    mAudio = nullptr;
    mFrameBuffer.clear();
    mFinished = true;
    mWidth = 0;
    mHeight = 0;
    mCurrentFrame = 0;
    mTotalFrames = 0;
}

bool TheoraMoviePlayer::TickFrame() {
    if (mFinished) {
        return false;
    }

    mCurrentFrame++;
    if (mCurrentFrame >= mTotalFrames) {
        mFinished = true;
        return false;
    }

    return true;
}

void TheoraMoviePlayer::RenderFrame() {
    if (mFinished || !mRenderer || mTexture == kInvalidTextureHandle) {
        return;
    }

    HalVertex verts[4];
    f32 w = static_cast<f32>(mWidth > 0 ? mWidth : 640);
    f32 h = static_cast<f32>(mHeight > 0 ? mHeight : 480);

    verts[0].x = 0.0f; verts[0].y = 0.0f; verts[0].z = 0.0f; verts[0].w = 1.0f;
    verts[0].diff = 0xFFFFFFFF; verts[0].spec = 0; verts[0].u = 0.0f; verts[0].v = 0.0f;

    verts[1].x = w;    verts[1].y = 0.0f; verts[1].z = 0.0f; verts[1].w = 1.0f;
    verts[1].diff = 0xFFFFFFFF; verts[1].spec = 0; verts[1].u = 1.0f; verts[1].v = 0.0f;

    verts[2].x = w;    verts[2].y = h;    verts[2].z = 0.0f; verts[2].w = 1.0f;
    verts[2].diff = 0xFFFFFFFF; verts[2].spec = 0; verts[2].u = 1.0f; verts[2].v = 1.0f;

    verts[3].x = 0.0f; verts[3].y = h;    verts[3].z = 0.0f; verts[3].w = 1.0f;
    verts[3].diff = 0xFFFFFFFF; verts[3].spec = 0; verts[3].u = 0.0f; verts[3].v = 1.0f;

    mRenderer->DrawQuad(verts, mTexture, RENDER_NONE, 0xFFFFFFFF);
}

} // namespace gta2::hal
