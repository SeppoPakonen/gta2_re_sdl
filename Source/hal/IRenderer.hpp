#pragma once
#include "HalTypes.hpp"

namespace gta2::hal {

class IWindowSystem;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool Initialize(IWindowSystem* pWindow) = 0;
    virtual void Shutdown() = 0;

    virtual void BeginScene() = 0;
    virtual void EndScene() = 0;
    virtual void Clear(u32 colorARGB) = 0;
    virtual void SetViewport(s32 x, s32 y, s32 width, s32 height) = 0;

    // Primitives
    virtual void DrawTriangle(const HalVertex* verts, TextureHandle texture, u32 flags, u32 diffuseColor) = 0;
    virtual void DrawQuad(const HalVertex* verts, TextureHandle texture, u32 flags, u32 diffuseColor) = 0;

    // Texture Management
    virtual TextureHandle RegisterTexture(const TextureDesc& desc) = 0;
    virtual void UpdateTexture(TextureHandle handle, const void* pPixels, const u16* pPalette) = 0;
    virtual void UnregisterTexture(TextureHandle handle) = 0;

    // Palette & Gamma
    virtual void SetPalette(TextureHandle texture, const u16* pPalette) = 0;
    virtual void SetGamma(f32 gamma) = 0;
};

} // namespace gta2::hal