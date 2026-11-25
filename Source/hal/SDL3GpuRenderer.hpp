#pragma once

#if defined(GTA2_USE_SDL3)

#include "IRenderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace gta2::hal {

class SDL3GpuRenderer : public IRenderer {
private:
    SDL_GPUDevice*        mDevice   = nullptr;
    SDL_Window*           mWindow   = nullptr;
    SDL_GPUGraphicsPipeline* mPipeline = nullptr;
    SDL_GPUCommandBuffer* mCmd      = nullptr;
    SDL_GPURenderPass*    mPass     = nullptr;

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
    void UpdateTexture(TextureHandle handle, const void* pPixels, const u16* pPalette,
                       const u32* pPaletteRGBA = nullptr, u16 width = 0, u16 height = 0) override;
    void UnregisterTexture(TextureHandle handle) override;

    void SetPalette(TextureHandle texture, const u16* pPalette, const u32* pPaletteRGBA = nullptr) override;
    void SetGamma(f32 gamma) override;
};

} // namespace gta2::hal

#endif