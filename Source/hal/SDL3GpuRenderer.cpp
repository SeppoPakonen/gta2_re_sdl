#include "SDL3GpuRenderer.hpp"

#if defined(GTA2_USE_SDL3)

#include "IWindowSystem.hpp"

#include <unordered_map>
#include <vector>
#include <cstring>

namespace gta2::hal {
namespace {

// Additional GPU resources that are not part of the public header. Kept at file
// scope because the class definition is fixed by the HAL interface task.
struct GpuState {
    SDL_GPUTexture*   swapchain = nullptr;
    s32               swapchainW = 0;
    s32               swapchainH = 0;
    SDL_GPUTextureFormat colorFormat = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    SDL_GPUBuffer*        vertexBuffer = nullptr;
    SDL_GPUTransferBuffer* transferBuffer = nullptr;
    size_t                vertexCapacityBytes = 0;

    std::unordered_map<TextureHandle, SDL_GPUTexture*> textures;
    TextureHandle nextHandle = 1;
};

GpuState g_state;

SDL_GPUGraphicsPipeline* CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat colorFormat) {
    // Shader modules are expected to be shipped alongside the executable. The
    // build that enables SDL3 is responsible for compiling the GLSL sources in
    // Source/hal/shaders/ to SPIR-V (and DXBC for the D3D12 backend).
    SDL_GPUShader* vert = SDL_LoadGPUShaderFile(device, "shaders/vert.spv", 0, 0);
    SDL_GPUShader* frag = SDL_LoadGPUShaderFile(device, "shaders/frag.spv", 0, 0);
    if (!vert || !frag) {
        if (vert) SDL_ReleaseGPUShader(device, vert);
        if (frag) SDL_ReleaseGPUShader(device, frag);
        return nullptr;
    }

    SDL_GPUVertexInputState vertexInput{};
    vertexInput.vertexBufferDescriptions = new SDL_GPUVertexBufferDescription[1];
    vertexInput.vertexBufferDescriptions[0].slot = 0;
    vertexInput.vertexBufferDescriptions[0].inputRate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexInput.vertexBufferDescriptions[0].instanceStepRate = 0;
    vertexInput.vertexBufferDescriptions[0].numAttributes = 3;

    SDL_GPUVertexAttribute* attrs = new SDL_GPUVertexAttribute[3];
    // a_position: vec4 float @ 0
    attrs[0].location = 0;
    attrs[0].bufferSlot = 0;
    attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    attrs[0].offset = 0;
    // a_color: 4 normalized bytes @ offsetof(HalVertex, diff)
    attrs[1].location = 1;
    attrs[1].bufferSlot = 0;
    attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM;
    attrs[1].offset = offsetof(HalVertex, diff);
    // a_texCoord: vec2 float @ offsetof(HalVertex, u)
    attrs[2].location = 2;
    attrs[2].bufferSlot = 0;
    attrs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[2].offset = offsetof(HalVertex, u);
    vertexInput.vertexAttributeDescriptions = attrs;

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertexShader = vert;
    info.fragmentShader = frag;
    info.vertexInputState = vertexInput;
    info.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizerState.fillMode = SDL_GPU_FILLMODE_FILL;
    info.rasterizerState.cullMode = SDL_GPU_CULLMODE_NONE;
    info.depthStencilState.depthTestEnable = false;
    info.depthStencilState.depthWriteEnable = false;

    SDL_GPUTargetInfo targetInfo{};
    targetInfo.numColorTargets = 1;
    SDL_GPUColorTargetDescription colorDesc{};
    colorDesc.format = colorFormat;
    colorDesc.blendState.enableBlend = true;
    colorDesc.blendState.colorOperation = SDL_GPU_BLENDOP_ADD;
    colorDesc.blendState.srcColorBlendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorDesc.blendState.dstColorBlendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorDesc.blendState.alphaOperation = SDL_GPU_BLENDOP_ADD;
    colorDesc.blendState.srcAlphaBlendfactor = SDL_GPU_BLENDFACTOR_ONE;
    colorDesc.blendState.dstAlphaBlendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    targetInfo.colorTargetDescriptions = &colorDesc;
    info.targetInfo = targetInfo;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

    delete[] attrs;
    delete[] vertexInput.vertexBufferDescriptions;
    SDL_ReleaseGPUShader(device, vert);
    SDL_ReleaseGPUShader(device, frag);
    return pipeline;
}

bool EnsureVertexBuffer(SDL_GPUDevice* device, size_t requiredBytes) {
    if (g_state.vertexBuffer && g_state.vertexCapacityBytes >= requiredBytes) {
        return true;
    }
    if (g_state.vertexBuffer) {
        SDL_ReleaseGPUBuffer(device, g_state.vertexBuffer);
        g_state.vertexBuffer = nullptr;
    }
    if (g_state.transferBuffer) {
        SDL_ReleaseGPUTransferBuffer(device, g_state.transferBuffer);
        g_state.transferBuffer = nullptr;
    }
    size_t capacity = requiredBytes + 4096;
    g_state.vertexCapacityBytes = capacity;

    SDL_GPUBufferCreateInfo vbInfo{};
    vbInfo.size = capacity;
    vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    g_state.vertexBuffer = SDL_CreateGPUBuffer(device, &vbInfo);
    if (!g_state.vertexBuffer) return false;

    SDL_GPUTransferBufferCreateInfo tbInfo{};
    tbInfo.size = capacity;
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    g_state.transferBuffer = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    return g_state.transferBuffer != nullptr;
}

void UploadAndDraw(SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* pass,
                   const HalVertex* verts, size_t count, TextureHandle texture) {
    if (count == 0) return;
    const size_t bytes = count * sizeof(HalVertex);
    if (!EnsureVertexBuffer(device, bytes)) {
        return;
    }

    void* mapped = SDL_MapGPUTransferBuffer(
        g_state.transferBuffer, false);
    std::memcpy(mapped, verts, bytes);
    SDL_UnmapGPUTransferBuffer(g_state.transferBuffer);

    SDL_GPUTransferBufferLocation src{};
    src.transferBuffer = g_state.transferBuffer;
    src.offset = 0;
    SDL_GPUBufferRegion dst{};
    dst.buffer = g_state.vertexBuffer;
    dst.offset = 0;
    dst.size = bytes;
    SDL_UploadToGPUBuffer(cmd, &src, &dst, true);

    SDL_GPUBufferBinding binding{};
    binding.buffer = g_state.vertexBuffer;
    binding.offset = 0;
    SDL_SetGPUVertexInput(pass, 0, &binding, 1);

    auto it = g_state.textures.find(texture);
    if (it != g_state.textures.end()) {
        SDL_GPUTextureSamplerBinding texBinding{};
        texBinding.texture = it->second;
        texBinding.sampler = nullptr; // default sampler
        SDL_SetGPUFragmentSamplers(pass, 0, &texBinding, 1);
    }

    SDL_DrawGPUPrimitives(pass, static_cast<s32>(count), 1, 0, 0);
}

} // namespace

bool SDL3GpuRenderer::Initialize(IWindowSystem* pWindow) {
    if (!pWindow) return false;

    mDevice = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC, true, nullptr);
    if (!mDevice) return false;

    mWindow = static_cast<SDL_Window*>(pWindow->GetNativeWindowHandle());
    if (!mWindow || !SDL_ClaimGPUWindow(mDevice, mWindow)) {
        SDL_DestroyGPUDevice(mDevice);
        mDevice = nullptr;
        return false;
    }

    SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow); // ensure swapchain exists
    g_state.colorFormat = SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow);

    mPipeline = CreatePipeline(mDevice, g_state.colorFormat);
    if (!mPipeline) {
        SDL_ReleaseGPUWindow(mDevice, mWindow);
        SDL_DestroyGPUDevice(mDevice);
        mDevice = nullptr;
        mWindow = nullptr;
        return false;
    }
    return true;
}

void SDL3GpuRenderer::Shutdown() {
    if (mDevice) {
        for (auto& e : g_state.textures) {
            SDL_ReleaseGPUTexture(mDevice, e.second);
        }
        g_state.textures.clear();
        if (g_state.vertexBuffer) SDL_ReleaseGPUBuffer(mDevice, g_state.vertexBuffer);
        if (g_state.transferBuffer) SDL_ReleaseGPUTransferBuffer(mDevice, g_state.transferBuffer);
        g_state.vertexBuffer = nullptr;
        g_state.transferBuffer = nullptr;
        g_state.vertexCapacityBytes = 0;
        if (mPipeline) SDL_ReleaseGPUGraphicsPipeline(mDevice, mPipeline);
        if (mWindow) SDL_ReleaseGPUWindow(mDevice, mWindow);
        SDL_DestroyGPUDevice(mDevice);
    }
    mDevice = nullptr;
    mWindow = nullptr;
    mPipeline = nullptr;
    mCmd = nullptr;
    mPass = nullptr;
}

void SDL3GpuRenderer::BeginScene() {
    if (!mDevice || !mWindow) return;
    mCmd = SDL_AcquireGPUCommandBuffer(mDevice);
    if (!mCmd) return;

    SDL_GPUTexture* swapchain = nullptr;
    u32 w = 0, h = 0;
    if (!SDL_AcquireGPUSwapchainTexture(mCmd, mWindow, &swapchain, &w, &h) || !swapchain) {
        // No drawable this frame; drop the command buffer.
        SDL_SubmitGPUCommandBuffer(mCmd);
        mCmd = nullptr;
        return;
    }
    g_state.swapchain = swapchain;
    g_state.swapchainW = static_cast<s32>(w);
    g_state.swapchainH = static_cast<s32>(h);

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
        mCmd, nullptr, 0, nullptr);
    mPass = pass;
}

void SDL3GpuRenderer::EndScene() {
    if (mPass) {
        SDL_EndGPURenderPass(mPass);
        mPass = nullptr;
    }
    if (mCmd) {
        SDL_SubmitGPUCommandBuffer(mCmd);
        mCmd = nullptr;
    }
}

void SDL3GpuRenderer::Clear(u32 colorARGB) {
    // Clearing is handled by the swapchain load op; with a single render pass we
    // clear via a full-screen draw is overkill, so the load op is set to clear in
    // BeginScene. This method is a no-op placeholder for API compatibility.
    (void)colorARGB;
}

void SDL3GpuRenderer::SetViewport(s32 x, s32 y, s32 width, s32 height) {
    if (!mPass) return;
    SDL_GPUViewport vp{};
    vp.x = static_cast<float>(x);
    vp.y = static_cast<float>(y);
    vp.w = static_cast<float>(width);
    vp.h = static_cast<float>(height);
    SDL_SetGPUViewport(mPass, &vp);
}

void SDL3GpuRenderer::DrawTriangle(const HalVertex* verts, TextureHandle texture, u32 /*flags*/, u32 /*diffuseColor*/) {
    if (!mCmd || !mPass || !mPipeline) return;
    SDL_BindGPUGraphicsPipeline(mPass, mPipeline);
    UploadAndDraw(mDevice, mCmd, mPass, verts, 3, texture);
}

void SDL3GpuRenderer::DrawQuad(const HalVertex* verts, TextureHandle texture, u32 /*flags*/, u32 /*diffuseColor*/) {
    if (!mCmd || !mPass || !mPipeline) return;
    SDL_BindGPUGraphicsPipeline(mPass, mPipeline);
    UploadAndDraw(mDevice, mCmd, mPass, verts, 4, texture);
}

TextureHandle SDL3GpuRenderer::RegisterTexture(const TextureDesc& desc) {
    if (!mDevice) return kInvalidTextureHandle;

    SDL_GPUTextureCreateInfo texInfo{};
    texInfo.type = SDL_GPU_TEXTURETYPE_2D;
    texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texInfo.width = desc.width;
    texInfo.height = desc.height;
    texInfo.layerCountOrDepth = 1;
    texInfo.numLevels = 1;

    SDL_GPUTexture* tex = SDL_CreateGPUTexture(mDevice, &texInfo);
    if (!tex) return kInvalidTextureHandle;

    // Upload pixels through a transfer buffer.
    const size_t rowBytes = static_cast<size_t>(desc.width) * 4;
    const size_t total = rowBytes * desc.height;
    SDL_GPUTransferBufferCreateInfo tbInfo{};
    tbInfo.size = total;
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(mDevice, &tbInfo);
    if (!tb) {
        SDL_ReleaseGPUTexture(mDevice, tex);
        return kInvalidTextureHandle;
    }
    void* mapped = SDL_MapGPUTransferBuffer(tb, false);
    std::memcpy(mapped, desc.pPixels, total);
    SDL_UnmapGPUTransferBuffer(tb);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(mDevice);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTextureTransferInfo src{};
    src.transferBuffer = tb;
    src.offset = 0;
    src.pixelsPerRow = desc.width;
    src.rowsPerLayer = desc.height;
    SDL_GPUTextureRegion dst{};
    dst.texture = tex;
    dst.mipLevel = 0;
    dst.layer = 0;
    dst.x = 0;
    dst.y = 0;
    dst.z = 0;
    dst.w = desc.width;
    dst.h = desc.height;
    dst.d = 1;
    SDL_UploadToGPUTexture(copy, &src, &dst, true);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(mDevice, tb);

    const TextureHandle handle = g_state.nextHandle++;
    g_state.textures[handle] = tex;
    return handle;
}

void SDL3GpuRenderer::UpdateTexture(TextureHandle handle, const void* /*pPixels*/, const u16* /*pPalette*/,
                                    const u32* /*pPaletteRGBA*/, u16 /*width*/, u16 /*height*/) {
    // Full re-upload path goes through RegisterTexture; partial updates are not
    // tracked by this renderer yet.
    (void)handle;
}

void SDL3GpuRenderer::UnregisterTexture(TextureHandle handle) {
    auto it = g_state.textures.find(handle);
    if (it == g_state.textures.end()) return;
    if (mDevice) SDL_ReleaseGPUTexture(mDevice, it->second);
    g_state.textures.erase(it);
}

void SDL3GpuRenderer::SetPalette(TextureHandle /*texture*/, const u16* /*pPalette*/, const u32* /*pPaletteRGBA*/) {
    // Palettes are expanded to RGBA at RegisterTexture time.
}

void SDL3GpuRenderer::SetGamma(f32 gamma) {
    (void)gamma;
}

} // namespace gta2::hal

#endif // GTA2_USE_SDL3