#pragma once

#include "IRenderer.hpp"
#include <unordered_map>
#include <vector>

namespace gta2::hal {

// Cross-platform OpenGL 2.1 / OpenGL ES 2.0 / WebGL 1.0 renderer.
//
// No OpenGL headers are included here: the GL entry points and constants are
// declared in OpenGLRenderer.cpp and resolved at runtime through a proc loader
// (e.g. SDL_GL_GetProcAddress). This keeps the renderer buildable on any
// platform, including Emscripten/WebGL where there is no native GL library.
class OpenGLRenderer : public IRenderer {
public:
    bool Initialize(IWindowSystem* pWindow) override;
    void Shutdown() override;

    void BeginScene() override;
    void EndScene() override;
    void Clear(u32 colorARGB) override;
    void SetViewport(s32 x, s32 y, s32 width, s32 height) override;

    void DrawTriangle(const HalVertex* verts, TextureHandle texture, u32 flags, u32 diffuseColor) override;
    void DrawQuad(const HalVertex* verts, TextureHandle texture, u32 flags, u32 diffuseColor) override;

    TextureHandle RegisterTexture(const TextureDesc& desc) override;
    void UpdateTexture(TextureHandle handle, const void* pPixels, const u16* pPalette) override;
    void UnregisterTexture(TextureHandle handle) override;

    void SetPalette(TextureHandle texture, const u16* pPalette) override;
    void SetGamma(f32 gamma) override;

    // Provide the GL entry-point resolver (e.g. SDL_GL_GetProcAddress). Must be
    // called after the GL context is current and before Initialize/rendering.
    static void SetProcLoader(void* (*pLoader)(const char* name));

private:
    void DrawPrimitive(const HalVertex* verts, int count, unsigned int mode,
                       TextureHandle texture, u32 flags);

    IWindowSystem* mWindow = nullptr;
    bool mReady = false;

    unsigned int mProgram = 0;
    int mPositionAttrib = -1;
    int mColorAttrib = -1;
    int mTexCoordAttrib = -1;
    int mProjUniform = -1;
    int mTextureUniform = -1;
    int mUseTextureUniform = -1;

    unsigned int mVbo = 0;

    std::unordered_map<TextureHandle, unsigned int> mTextures;
    TextureHandle mNextTextureHandle = 1;
};

} // namespace gta2::hal