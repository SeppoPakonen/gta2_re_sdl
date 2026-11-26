#if defined(GTA2_MODERN_BUILD)

// Modern build: the legacy GBH graphics DLL loader (gbh_graphics.hpp pulls in
// <windows.h>/ddraw through d3ddll) is only used by the VC6 byte-matching build.
// Here we provide a bridge that redirects every pgbh_* function pointer into the
// HAL renderer (gta2::hal::GetHal().pRenderer), so the hundreds of legacy call
// sites in Draw.cpp / MapRenderer.cpp / SpriteRenderer.cpp / KanjiFont.cpp keep
// working without modification.
#include "gbh_graphics.hpp"
#include "hal/HalContext.hpp"
#include "hal/IRenderer.hpp"
#include "gtx_0x106C.hpp"
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>

using gta2::hal::s32;
using gta2::hal::u8;
using gta2::hal::u16;
using gta2::hal::u32;

// Layout-compatible mirrors of the legacy gbh types (see d3ddll.hpp).
// STexture and Vert are defined in gbh_graphics.hpp.

// pgbh_* pointers defined for modern build
T_gbh_DrawQuad      pgbh_DrawQuad      = nullptr;
T_gbh_DrawTile      pgbh_DrawTile      = nullptr;
T_gbh_DrawTilePart  pgbh_DrawTilePart  = nullptr;
T_gbh_DrawTriangle  pgbh_DrawTriangle  = nullptr;
T_gbh_BeginScene    pgbh_BeginScene    = nullptr;
T_gbh_EndScene      pgbh_EndScene      = nullptr;
T_gbh_SetWindow     pgbh_SetWindow     = nullptr;

T_gbh_InitDLL          pgbh_InitDLL          = nullptr;
T_gbh_Init             pgbh_Init             = nullptr;
T_gbh_CloseDLL         pgbh_CloseDLL         = nullptr;
T_gbh_GetGlobals       pgbh_GetGlobals       = nullptr;
T_gbh_DrawQuadClipped  pgbh_DrawQuadClipped  = nullptr;
T_gbh_DrawFlatRect     pgbh_DrawFlatRect     = nullptr;
T_gbh_PrintBitmap      pgbh_PrintBitmap      = nullptr;
T_gbh_SetColourDepth   pgbh_SetColourDepth   = nullptr;
T_gbh_Plot             pgbh_Plot             = nullptr;
T_gbh_ConvertColour    pgbh_ConvertColour    = nullptr;
T_gbh_RegisterTexture  pgbh_RegisterTexture  = nullptr;
T_gbh_SetCamera        pgbh_SetCamera        = nullptr;
T_gbh_ResetLights      pgbh_ResetLights      = nullptr;
T_gbh_AddLight         pgbh_AddLight         = nullptr;
T_gbh_SetAmbient       pgbh_SetAmbient       = nullptr;
T_gbh_GetUsedCache     pgbh_GetUsedCache     = nullptr;
T_gbh_FreeTexture      pgbh_FreeTexture      = nullptr;
T_gbh_RegisterPalette  pgbh_RegisterPalette  = nullptr;
T_gbh_FreePalette      pgbh_FreePalette      = nullptr;
T_gbh_AssignPalette    pgbh_AssignPalette    = nullptr;
T_gbh_LockTexture      pgbh_LockTexture      = nullptr;
T_gbh_UnlockTexture    pgbh_UnlockTexture    = nullptr;
T_gbh_BeginLevel       pgbh_BeginLevel       = nullptr;
T_gbh_EndLevel         pgbh_EndLevel         = nullptr;

T_MakeScreenTable      pMakeScreenTable      = nullptr;
T_ConvertColourBank    pConvertColourBank    = nullptr;
T_SetShadeTableA       pSetShadeTableA       = nullptr;

T_gbh_InitImageTable   pgbh_InitImageTable   = nullptr;
T_gbh_FreeImageTable   pgbh_FreeImageTable   = nullptr;
T_gbh_LoadImage        pgbh_LoadImage        = nullptr;
T_gbh_BlitImage        pgbh_BlitImage        = nullptr;
T_gbh_BlitBuffer       pgbh_BlitBuffer       = nullptr;

s32 gGBH_GraphicsLoaded_7085E0 = 1;

// Modern TGA image registry: maps image_idx -> HAL TextureHandle
static std::vector<gta2::hal::TextureHandle> sBridgeImageHandles;
static std::vector<u16> sBridgeImageWidths;
static std::vector<u16> sBridgeImageHeights;
static std::vector<bool> sBridgeImageLoaded;
static s32 sNextBridgeImageIdx = 1;

static void Dummy_NoOp() {}
static s32  Dummy_Zero() { return 0; }
static void* Dummy_Null() { return nullptr; }

static gta2::hal::TextureHandle Bridge_TextureHandle(STexture* pTexture) {
    if (!pTexture) {
        return gta2::hal::kInvalidTextureHandle;
    }
    return static_cast<gta2::hal::TextureHandle>(
        reinterpret_cast<std::uintptr_t>(pTexture->field_1C_ptr));
}

static s32 Bridge_gbh_LoadImage(SImage* pImage) {
    if (!pImage) return -1;
    u8* pBuf = reinterpret_cast<u8*>(pImage);
    if (!pBuf) return -1;

    u8 idLength = pBuf[0];
    u8 colorMapType = pBuf[1];
    u8 imageType = pBuf[2];
    u16 firstEntryIndex = static_cast<u16>((pBuf[3] & 0xFF) | (pBuf[4] << 8));
    u16 colorMapLength = static_cast<u16>((pBuf[5] & 0xFF) | (pBuf[6] << 8));
    u8 colorMapEntrySize = pBuf[7];
    u16 width = static_cast<u16>((pBuf[12] & 0xFF) | (pBuf[13] << 8));
    u16 height = static_cast<u16>((pBuf[14] & 0xFF) | (pBuf[15] << 8));
    u8 bitsPerPixel = pBuf[16];

    (void)firstEntryIndex;
    if (width == 0 || height == 0) return -1;

    const void* pPixels = nullptr;
    const u16* pPalette565 = nullptr;
    const u32* pPaletteRGBA = nullptr;

    u32 headerSize = 18 + idLength;
    // colorMapEntrySize is bits per palette entry (e.g. 24); convert to bytes.
    u32 palEntryBytes = ((colorMapEntrySize > 0) ? colorMapEntrySize : 24) / 8;
    // The color map (if present) starts right after the header + image id.
    u32 colorMapStart = headerSize;
    u32 colorMapBytes = (colorMapType == 1) ? (colorMapLength * palEntryBytes) : 0;
    // Pixel data follows the color map.
    u32 pixOffset = colorMapStart + colorMapBytes;

    if (imageType == 1 || imageType == 9) {
        if (colorMapBytes > 0) {
            u16* pal565 = new u16[256]();
            u32* palRGBA = new u32[256]();
            for (u32 i = 0; i < colorMapLength; ++i) {
                u32 idx = firstEntryIndex + i;
                if (idx >= 256) break;
                u32 palPos = colorMapStart + (i * palEntryBytes);
                u8 b = pBuf[palPos];
                u8 g = pBuf[palPos + 1];
                u8 r = pBuf[palPos + 2];
                pal565[idx] = static_cast<u16>((static_cast<u32>(r) * 31 / 255) << 11) |
                             (static_cast<u32>(g) * 63 / 255) << 5 |
                             (static_cast<u32>(b) * 31 / 255);
                palRGBA[idx] = static_cast<u32>(r) | (static_cast<u32>(g) << 8) |
                              (static_cast<u32>(b) << 16) | 0xFF000000u;
            }
            pPalette565 = pal565;
            pPaletteRGBA = palRGBA;
        }
        pPixels = const_cast<const void*>(reinterpret_cast<const void*>(pBuf + pixOffset));
    }
    else if (imageType == 2 || imageType == 10) {
        u32 pixDataSize = static_cast<u32>(width) * height * (bitsPerPixel / 8);
        u32* rgbaBuf = new u32[width * height]();
        const u8* srcPixels = pBuf + pixOffset;
        if (bitsPerPixel == 16) {
            // Menu TGAs are ARGB1555 (descriptor alpha bit = 1): 1 alpha + 5-5-5.
            // Decoding as 565 read a 6-bit green that swallowed the alpha bit and
            // produced green banding. Treat the image as opaque.
            for (u32 i = 0; i < width * height; ++i) {
                u16 px = *reinterpret_cast<const u16*>(srcPixels + i * 2);
                u8 r = (px >> 10) & 0x1F; r = (r * 255 + 15) / 31;
                u8 g = (px >> 5) & 0x1F;  g = (g * 255 + 15) / 31;
                u8 b = px & 0x1F;         b = (b * 255 + 15) / 31;
                rgbaBuf[i] = static_cast<u32>(r) | (static_cast<u32>(g) << 8) |
                              (static_cast<u32>(b) << 16) | 0xFF000000u;
            }
        } else if (bitsPerPixel == 24) {
            for (u32 i = 0; i < width * height; ++i) {
                u8 b = srcPixels[i * 3];
                u8 g = srcPixels[i * 3 + 1];
                u8 r = srcPixels[i * 3 + 2];
                rgbaBuf[i] = static_cast<u32>(r) | (static_cast<u32>(g) << 8) |
                              (static_cast<u32>(b) << 16) | 0xFF000000u;
            }
        } else if (bitsPerPixel == 32) {
            for (u32 i = 0; i < width * height; ++i) {
                u8 b = srcPixels[i * 4];
                u8 g = srcPixels[i * 4 + 1];
                u8 r = srcPixels[i * 4 + 2];
                u8 a = srcPixels[i * 4 + 3];
                rgbaBuf[i] = static_cast<u32>(r) | (static_cast<u32>(g) << 8) |
                              (static_cast<u32>(b) << 16) | (static_cast<u32>(a) << 24);
            }
        }
        pPixels = rgbaBuf;
        bitsPerPixel = 32;
    }
    else {
        pPixels = const_cast<const void*>(reinterpret_cast<const void*>(pBuf + pixOffset));
    }

    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (!pRenderer) return -1;

    gta2::hal::TextureDesc desc{};
    desc.width = width;
    desc.height = height;
    desc.bitsPerPixel = bitsPerPixel;
    desc.pPixels = pPixels;
    desc.pPalette = pPalette565;
    desc.pPaletteRGBA = pPaletteRGBA;
    desc.bTransparent = true;

    gta2::hal::TextureHandle h = pRenderer->RegisterTexture(desc);
    if (h == gta2::hal::kInvalidTextureHandle) return -1;

    sBridgeImageHandles.resize(sNextBridgeImageIdx + 1);
    sBridgeImageWidths.resize(sNextBridgeImageIdx + 1);
    sBridgeImageHeights.resize(sNextBridgeImageIdx + 1);
    sBridgeImageLoaded.resize(sNextBridgeImageIdx + 1);
    sBridgeImageHandles[sNextBridgeImageIdx] = h;
    sBridgeImageWidths[sNextBridgeImageIdx] = width;
    sBridgeImageHeights[sNextBridgeImageIdx] = height;
    sBridgeImageLoaded[sNextBridgeImageIdx] = true;
    sNextBridgeImageIdx++;
    return sNextBridgeImageIdx - 1;
}

static s32 Bridge_gbh_BlitImage(s32 image_idx, s32 src_left, s32 src_top, s32 src_right, s32 src_bottom, s32 dst_x, s32 dst_y) {
    if (image_idx < 0 || image_idx >= (s32)sBridgeImageLoaded.size() || !sBridgeImageLoaded[image_idx]) {
        return -10;
    }
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (!pRenderer) return -10;
    gta2::hal::TextureHandle h = sBridgeImageHandles[image_idx];
    gta2::hal::HalVertex verts[4];
    // Pass pixel-space UVs; DrawPrimitive normalises by the texture size. (These
    // used to be pre-divided by width/height, which double-normalised and made
    // every blit sample only the left-most pixel column.) V is swapped because
    // the TGA origin is bottom-left, so the image rows are bottom-up.
    f32 u0 = static_cast<f32>(src_left);
    f32 u1 = static_cast<f32>(src_right);
    f32 v0 = static_cast<f32>(src_bottom);
    f32 v1 = static_cast<f32>(src_top);
    verts[0].x = static_cast<f32>(dst_x); verts[0].y = static_cast<f32>(dst_y);
    verts[0].z = 0.0f; verts[0].w = 1.0f; verts[0].u = u0; verts[0].v = v0;
    verts[1].x = static_cast<f32>(dst_x + src_right - src_left); verts[1].y = static_cast<f32>(dst_y);
    verts[1].z = 0.0f; verts[1].w = 1.0f; verts[1].u = u1; verts[1].v = v0;
    verts[2].x = static_cast<f32>(dst_x); verts[2].y = static_cast<f32>(dst_y + src_bottom - src_top);
    verts[2].z = 0.0f; verts[2].w = 1.0f; verts[2].u = u0; verts[2].v = v1;
    verts[3].x = static_cast<f32>(dst_x + src_right - src_left); verts[3].y = static_cast<f32>(dst_y + src_bottom - src_top);
    verts[3].z = 0.0f; verts[3].w = 1.0f; verts[3].u = u1; verts[3].v = v1;
    // Vertex diffuse/specular colours must be initialised; the shader multiplies
    // the texture by the vertex colour, so leaving them as stack garbage renders
    // the image black.
    for (int i = 0; i < 4; ++i) {
        verts[i].diff = 0xFFFFFFFFu;
        verts[i].spec = 0;
    }
    pRenderer->DrawQuad(verts, h, 0x180, 0xFFFFFF);
    return 0;
}

static s32 Bridge_gbh_InitImageTable(s32 tableSize) {
    (void)tableSize;
    sBridgeImageHandles.clear();
    sBridgeImageWidths.clear();
    sBridgeImageHeights.clear();
    sBridgeImageLoaded.clear();
    sBridgeImageHandles.push_back(gta2::hal::kInvalidTextureHandle);
    sBridgeImageWidths.push_back(0);
    sBridgeImageHeights.push_back(0);
    sBridgeImageLoaded.push_back(false);
    sNextBridgeImageIdx = 1;
    return 0;
}

static void Bridge_gbh_FreeImageTable() {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (!pRenderer) return;
    for (size_t i = 1; i < sBridgeImageHandles.size(); ++i) {
        if (sBridgeImageLoaded[i]) {
            pRenderer->UnregisterTexture(sBridgeImageHandles[i]);
        }
    }
    sBridgeImageHandles.clear();
    sBridgeImageWidths.clear();
    sBridgeImageHeights.clear();
    sBridgeImageLoaded.clear();
    sBridgeImageHandles.push_back(gta2::hal::kInvalidTextureHandle);
    sBridgeImageWidths.push_back(0);
    sBridgeImageHeights.push_back(0);
    sBridgeImageLoaded.push_back(false);
    sNextBridgeImageIdx = 1;
}

static void Bridge_gbh_DrawQuad(s32 flags, STexture* pTexture, Vert* pVerts, s32 baseColour) {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (!pRenderer || !pVerts) return;
    Vert quadVerts[4];
    std::memcpy(quadVerts, pVerts, 4 * sizeof(Vert));
    if (!(flags & 0x2000)) {
        u8 c = static_cast<u8>(baseColour);
        u32 diff = 0xFF000000 | (c << 16) | (c << 8) | c;
        for (int i = 0; i < 4; ++i) {
            quadVerts[i].diff = diff;
        }
    }
    pRenderer->DrawQuad(reinterpret_cast<const gta2::hal::HalVertex*>(quadVerts),
                        Bridge_TextureHandle(pTexture),
                        static_cast<u32>(flags),
                        static_cast<u32>(baseColour));
}

static s32 Bridge_gbh_DrawTilePart(u32 flags, STexture* pTexture, Vert* pData, s32 diffuseColour) {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (!pRenderer || !pData) return 0;

    Vert tileVerts[4];
    std::memcpy(tileVerts, pData, 4 * sizeof(Vert));

    u32 oldFlags = flags;

    if (!(flags & 0x4000)) {
        tileVerts[0].u = 0.5f;
        tileVerts[0].v = 0.5f;
        tileVerts[1].u = 63.499901f;
        tileVerts[1].v = 0.5f;
        tileVerts[2].u = 63.499901f;
        tileVerts[2].v = 63.499901f;
        tileVerts[3].u = 0.5f;
        tileVerts[3].v = 63.499901f;
    }

    struct UV { float u, v; };
    UV uvs[4];
    for (int i = 0; i < 4; i++) {
        uvs[i].u = tileVerts[i].u;
        uvs[i].v = tileVerts[i].v;
    }

    bool updated = false;
    switch (flags & 0x60) {
        case 0x20:
            tileVerts[0].u = uvs[3].u; tileVerts[0].v = uvs[3].v;
            tileVerts[1].u = uvs[0].u; tileVerts[1].v = uvs[0].v;
            tileVerts[2].u = uvs[1].u; tileVerts[2].v = uvs[1].v;
            tileVerts[3].u = uvs[2].u; tileVerts[3].v = uvs[2].v;
            updated = true;
            break;
        case 0x40:
            oldFlags = flags ^ 0x18;
            break;
        case 0x60:
            tileVerts[0].u = uvs[1].u; tileVerts[0].v = uvs[1].v;
            tileVerts[1].u = uvs[2].u; tileVerts[1].v = uvs[2].v;
            tileVerts[2].u = uvs[3].u; tileVerts[2].v = uvs[3].v;
            tileVerts[3].u = uvs[0].u; tileVerts[3].v = uvs[0].v;
            updated = true;
            break;
    }

    if (updated) {
        for (int i = 0; i < 4; i++) {
            uvs[i].u = tileVerts[i].u;
            uvs[i].v = tileVerts[i].v;
        }
    }

    // Flip up/down
    if (oldFlags & 8) {
        tileVerts[0].u = uvs[1].u; tileVerts[0].v = uvs[1].v;
        tileVerts[1].u = uvs[0].u; tileVerts[1].v = uvs[0].v;
        tileVerts[2].u = uvs[3].u; tileVerts[2].v = uvs[3].v;
        tileVerts[3].u = uvs[2].u; tileVerts[3].v = uvs[2].v;
        if (oldFlags & 0x10) {
            for (int i = 0; i < 4; i++) {
                uvs[i].u = tileVerts[i].u;
                uvs[i].v = tileVerts[i].v;
            }
        }
    }

    // Flip left/right
    if (oldFlags & 0x10) {
        tileVerts[0].u = uvs[3].u; tileVerts[0].v = uvs[3].v;
        tileVerts[1].u = uvs[2].u; tileVerts[1].v = uvs[2].v;
        tileVerts[2].u = uvs[1].u; tileVerts[2].v = uvs[1].v;
        tileVerts[3].u = uvs[0].u; tileVerts[3].v = uvs[0].v;
    }

    if (!(flags & 0x2000)) {
        u8 c = static_cast<u8>(diffuseColour);
        u32 diff = 0xFF000000 | (c << 16) | (c << 8) | c;
        for (int i = 0; i < 4; ++i) {
            tileVerts[i].diff = diff;
        }
    }

    pRenderer->DrawQuad(reinterpret_cast<const gta2::hal::HalVertex*>(tileVerts),
                        Bridge_TextureHandle(pTexture),
                        oldFlags,
                        static_cast<u32>(diffuseColour));
    return 0;
}

static s32 Bridge_gbh_DrawTile(u32 flags, STexture* pTexture, Vert* pData, u8 diffuseColour) {
    return Bridge_gbh_DrawTilePart(flags, pTexture, pData, diffuseColour);
}

static void Bridge_gbh_DrawTriangle(s32 triFlags, STexture* pTexture, Vert* pVerts, u8 diffuseColour) {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (!pRenderer || !pVerts) return;
    Vert triVerts[3];
    std::memcpy(triVerts, pVerts, 3 * sizeof(Vert));
    if (!(triFlags & 0x2000)) {
        u8 c = diffuseColour;
        u32 diff = 0xFF000000 | (c << 16) | (c << 8) | c;
        for (int i = 0; i < 3; ++i) {
            triVerts[i].diff = diff;
        }
    }
    pRenderer->DrawTriangle(reinterpret_cast<const gta2::hal::HalVertex*>(triVerts),
                            Bridge_TextureHandle(pTexture),
                            static_cast<u32>(triFlags),
                            static_cast<u32>(diffuseColour));
}

static void Bridge_gbh_BeginScene() {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (pRenderer) pRenderer->BeginScene();
}

static void Bridge_gbh_EndScene() {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (pRenderer) pRenderer->EndScene();
}

static void Bridge_gbh_SetWindow(float left, float top, float right, float bottom) {
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (pRenderer) {
        pRenderer->SetViewport(static_cast<s32>(left), static_cast<s32>(top),
                               static_cast<s32>(right - left), static_cast<s32>(bottom - top));
    }
}

struct BridgePalette {
    u16 colors565[256];
    u32 colorsRGBA[256];
    bool loaded = false;
};

static std::vector<BridgePalette> sBridgePalettes;

static u32 Bridge_gbh_RegisterPalette(s32 palId, DWORD* pData) {
    if (palId < 0 || !pData) return 0;
    if (static_cast<size_t>(palId) >= sBridgePalettes.size()) {
        sBridgePalettes.resize(palId + 64);
    }
    BridgePalette& pal = sBridgePalettes[palId];
    pal.loaded = true;
    for (int i = 0; i < 256; ++i) {
        DWORD raw = pData[i * 64];
        u8 r = (raw >> 16) & 0xFF;
        u8 g = (raw >> 8) & 0xFF;
        u8 b = raw & 0xFF;
        u8 a = (i == 0) ? 0 : 255;
        // In memory on x86 little-endian: byte 0=R, 1=G, 2=B, 3=A
        pal.colorsRGBA[i] = static_cast<u32>(r) | (static_cast<u32>(g) << 8) |
                            (static_cast<u32>(b) << 16) | (static_cast<u32>(a) << 24);
        u16 r5 = (r * 31) / 255;
        u16 g6 = (g * 63) / 255;
        u16 b5 = (b * 31) / 255;
        pal.colors565[i] = (r5 << 11) | (g6 << 5) | b5;
    }
    return static_cast<u32>(palId);
}

static const BridgePalette* GetOrRegisterPalette(s32 palId) {
    if (palId < 0) return nullptr;
    if (static_cast<size_t>(palId) < sBridgePalettes.size() && sBridgePalettes[palId].loaded) {
        return &sBridgePalettes[palId];
    }
    if (gGtx_0x106C_703DD4) {
        DWORD* pData = reinterpret_cast<DWORD*>(gGtx_0x106C_703DD4->GetPalData_5AA6A0(static_cast<u16>(palId)));
        if (pData) {
            Bridge_gbh_RegisterPalette(palId, pData);
            return &sBridgePalettes[palId];
        }
    }
    return nullptr;
}

static STexture* Bridge_gbh_RegisterTexture(s32 width, s32 height, BYTE* pData, s32 pal_idx, char_type flag) {
    STexture* tex = new STexture();
    std::memset(tex, 0, sizeof(STexture));
    tex->field_E_width = static_cast<u16>(width);
    tex->field_10_height = static_cast<u16>(height);
    tex->field_14_original_pixel_data_ptr = pData;
    tex->field_4_pal_is_trans = static_cast<u16>(flag);
    const BridgePalette* pal = (pal_idx >= 0) ? GetOrRegisterPalette(pal_idx) : nullptr;
    if (pal) {
        tex->field_18_pPaltData = const_cast<u16*>(pal->colors565);
        tex->field_12_bPalIsValid = 1;
    }
    gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
    if (pRenderer) {
        gta2::hal::TextureDesc desc{};
        desc.width = static_cast<gta2::hal::u16>(width);
        desc.height = static_cast<gta2::hal::u16>(height);
        desc.bitsPerPixel = 8;
        desc.pPixels = pData;
        desc.pPalette = pal ? pal->colors565 : nullptr;
        desc.pPaletteRGBA = pal ? pal->colorsRGBA : nullptr;
        desc.bTransparent = (flag != 0);
        // Sprite/mask pixel data is stored in 256-byte-wide rows (see
        // sprite_index::CopyPixels_5ABB00, idx = ypos << 8); only the left
        // field_4_width bytes of each row are the sprite. Use a 256 stride for
        // these page-based textures instead of the packed width.
        desc.srcStride = (width <= 256) ? 256u : static_cast<gta2::hal::u32>(width);
        gta2::hal::TextureHandle h = pRenderer->RegisterTexture(desc);
        tex->field_0_id = static_cast<u16>(h);
        tex->field_1C_ptr = reinterpret_cast<void*>(static_cast<std::uintptr_t>(h));
    }
    return tex;
}

static void Bridge_gbh_FreeTexture(STexture* pTexture) {
    if (pTexture) {
        gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
        gta2::hal::TextureHandle h = Bridge_TextureHandle(pTexture);
        if (pRenderer && h != gta2::hal::kInvalidTextureHandle) {
            pRenderer->UnregisterTexture(h);
        }
        delete pTexture;
    }
}

static void* Bridge_gbh_LockTexture(STexture* pTexture) {
    if (pTexture) {
        pTexture->field_8_locked_pixels_ptr = pTexture->field_14_original_pixel_data_ptr;
        return pTexture->field_8_locked_pixels_ptr;
    }
    return nullptr;
}

static STexture* Bridge_gbh_UnlockTexture(STexture* pTexture) {
    if (pTexture) {
        gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
        gta2::hal::TextureHandle h = Bridge_TextureHandle(pTexture);
        if (pRenderer && h != gta2::hal::kInvalidTextureHandle && pTexture->field_14_original_pixel_data_ptr) {
            const u32* pPalRGBA = nullptr;
            if (pTexture->field_18_pPaltData) {
                for (const auto& pal : sBridgePalettes) {
                    if (pal.colors565 == pTexture->field_18_pPaltData) {
                        pPalRGBA = pal.colorsRGBA;
                        break;
                    }
                }
            }
            pRenderer->UpdateTexture(h, pTexture->field_14_original_pixel_data_ptr,
                                    pTexture->field_18_pPaltData, pPalRGBA,
                                    pTexture->field_E_width, pTexture->field_10_height);
        }
    }
    return pTexture;
}

static void Bridge_gbh_AssignPalette(STexture* pTexture, s32 palId) {
    if (pTexture) {
        pTexture->field_12_bPalIsValid = 1;
        const BridgePalette* pal = GetOrRegisterPalette(palId);
        if (pal) {
            pTexture->field_18_pPaltData = const_cast<u16*>(pal->colors565);
            gta2::hal::IRenderer* pRenderer = gta2::hal::GetHal().pRenderer;
            if (pRenderer) {
                pRenderer->SetPalette(Bridge_TextureHandle(pTexture), pal->colors565, pal->colorsRGBA);
            }
        }
    }
}

// Bind every pgbh_* pointer to its HAL bridge. Called by the modern engine
// startup (EngineRunner) instead of GBH_GraphicsLoadDll_5EA680.
void GBH_GraphicsInit_Modern() {
    pgbh_DrawQuad     = Bridge_gbh_DrawQuad;
    pgbh_DrawTile     = Bridge_gbh_DrawTile;
    pgbh_DrawTilePart = Bridge_gbh_DrawTilePart;
    pgbh_DrawTriangle = Bridge_gbh_DrawTriangle;
    pgbh_BeginScene   = Bridge_gbh_BeginScene;
    pgbh_EndScene     = Bridge_gbh_EndScene;
    pgbh_SetWindow    = Bridge_gbh_SetWindow;
    pgbh_RegisterTexture = Bridge_gbh_RegisterTexture;
    pgbh_FreeTexture     = Bridge_gbh_FreeTexture;
    pgbh_LockTexture     = reinterpret_cast<T_gbh_LockTexture>(Bridge_gbh_LockTexture);
    pgbh_UnlockTexture   = Bridge_gbh_UnlockTexture;
    pgbh_AssignPalette   = Bridge_gbh_AssignPalette;

    pgbh_RegisterPalette = reinterpret_cast<T_gbh_RegisterPalette>(Bridge_gbh_RegisterPalette);
    pgbh_FreePalette      = reinterpret_cast<T_gbh_FreePalette>(Dummy_NoOp);
    pgbh_InitDLL          = reinterpret_cast<T_gbh_InitDLL>(Dummy_Zero);
    pgbh_Init             = reinterpret_cast<T_gbh_Init>(Dummy_Zero);
    pgbh_CloseDLL         = reinterpret_cast<T_gbh_CloseDLL>(Dummy_NoOp);
    pgbh_GetGlobals       = reinterpret_cast<T_gbh_GetGlobals>(Dummy_Null);
    pgbh_DrawQuadClipped  = reinterpret_cast<T_gbh_DrawQuadClipped>(Dummy_Zero);
    pgbh_DrawFlatRect     = reinterpret_cast<T_gbh_DrawFlatRect>(Dummy_Zero);
    pgbh_PrintBitmap      = reinterpret_cast<T_gbh_PrintBitmap>(Dummy_Zero);
    pgbh_SetColourDepth   = reinterpret_cast<T_gbh_SetColourDepth>(Dummy_Zero);
    pgbh_Plot             = reinterpret_cast<T_gbh_Plot>(Dummy_NoOp);
    pgbh_ConvertColour    = reinterpret_cast<T_gbh_ConvertColour>(Dummy_Zero);
    pgbh_SetCamera        = reinterpret_cast<T_gbh_SetCamera>(Dummy_NoOp);
    pgbh_ResetLights      = reinterpret_cast<T_gbh_ResetLights>(Dummy_NoOp);
    pgbh_AddLight         = reinterpret_cast<T_gbh_AddLight>(Dummy_Zero);
    pgbh_SetAmbient       = reinterpret_cast<T_gbh_SetAmbient>(Dummy_NoOp);
    pgbh_GetUsedCache     = reinterpret_cast<T_gbh_GetUsedCache>(Dummy_Zero);
    pgbh_RegisterPalette  = reinterpret_cast<T_gbh_RegisterPalette>(Dummy_Zero);
    pgbh_FreePalette      = reinterpret_cast<T_gbh_FreePalette>(Dummy_NoOp);
    pgbh_BeginLevel       = reinterpret_cast<T_gbh_BeginLevel>(Dummy_Zero);
    pgbh_EndLevel         = reinterpret_cast<T_gbh_EndLevel>(Dummy_NoOp);

    pMakeScreenTable      = reinterpret_cast<T_MakeScreenTable>(Dummy_NoOp);
    pConvertColourBank    = reinterpret_cast<T_ConvertColourBank>(Dummy_NoOp);
    pSetShadeTableA       = reinterpret_cast<T_SetShadeTableA>(Dummy_NoOp);

    pgbh_InitImageTable   = reinterpret_cast<T_gbh_InitImageTable>(Bridge_gbh_InitImageTable);
    pgbh_FreeImageTable   = reinterpret_cast<T_gbh_FreeImageTable>(Bridge_gbh_FreeImageTable);
    pgbh_LoadImage        = reinterpret_cast<T_gbh_LoadImage>(Bridge_gbh_LoadImage);
    pgbh_BlitImage        = reinterpret_cast<T_gbh_BlitImage>(Bridge_gbh_BlitImage);
    pgbh_BlitBuffer       = reinterpret_cast<T_gbh_BlitBuffer>(Dummy_Zero);
}

#else

#include "gbh_graphics.hpp"
#include "Function.hpp"
#include "Globals.hpp"
#include <stdio.h>

DEFINE_GLOBAL_INIT(T_gbh_InitDLL,         pgbh_InitDLL,         reinterpret_cast<T_gbh_InitDLL>(GBH_Graphics_Stub_5EA4D0),         0x626C54);
DEFINE_GLOBAL_INIT(T_gbh_Init,            pgbh_Init,            reinterpret_cast<T_gbh_Init>(GBH_Graphics_Stub_5EA4D0),            0x626C58);
DEFINE_GLOBAL_INIT(T_gbh_CloseDLL,        pgbh_CloseDLL,        static_cast<T_gbh_CloseDLL>(GBH_Graphics_Stub_5EA4D0),        0x626C5C);
DEFINE_GLOBAL_INIT(T_gbh_SetWindow,       pgbh_SetWindow,       reinterpret_cast<T_gbh_SetWindow>(GBH_Graphics_Stub_5EA4D0),       0x626C60);
DEFINE_GLOBAL_INIT(T_gbh_GetGlobals,      pgbh_GetGlobals,      reinterpret_cast<T_gbh_GetGlobals>(GBH_Graphics_Stub_5EA4D0),      0x626C64);
DEFINE_GLOBAL_INIT(T_gbh_DrawTile,        pgbh_DrawTile,        reinterpret_cast<T_gbh_DrawTile>(GBH_Graphics_Stub_5EA4D0),        0x626C68);
DEFINE_GLOBAL_INIT(T_gbh_DrawTilePart,    pgbh_DrawTilePart,    reinterpret_cast<T_gbh_DrawTilePart>(GBH_Graphics_Stub_5EA4D0),    0x626C6C);
DEFINE_GLOBAL_INIT(T_gbh_DrawQuad,        pgbh_DrawQuad,        reinterpret_cast<T_gbh_DrawQuad>(GBH_Graphics_Stub_5EA4D0),        0x626C70);
DEFINE_GLOBAL_INIT(T_gbh_DrawQuadClipped, pgbh_DrawQuadClipped, reinterpret_cast<T_gbh_DrawQuadClipped>(GBH_Graphics_Stub_5EA4D0), 0x626C74);
DEFINE_GLOBAL_INIT(T_gbh_DrawFlatRect,    pgbh_DrawFlatRect,    reinterpret_cast<T_gbh_DrawFlatRect>(GBH_Graphics_Stub_5EA4D0),    0x626C78);
DEFINE_GLOBAL_INIT(T_gbh_DrawTriangle,    pgbh_DrawTriangle,    reinterpret_cast<T_gbh_DrawTriangle>(GBH_Graphics_Stub_5EA4D0),    0x626C7C);
DEFINE_GLOBAL_INIT(T_gbh_PrintBitmap,     pgbh_PrintBitmap,     reinterpret_cast<T_gbh_PrintBitmap>(GBH_Graphics_Stub_5EA4D0),     0x626C80);
DEFINE_GLOBAL_INIT(T_gbh_SetColourDepth,  pgbh_SetColourDepth,  reinterpret_cast<T_gbh_SetColourDepth>(GBH_Graphics_Stub_5EA4D0),  0x626C84);
DEFINE_GLOBAL_INIT(T_gbh_Plot,            pgbh_Plot,            reinterpret_cast<T_gbh_Plot>(GBH_Graphics_Stub_5EA4D0),            0x626C88);
DEFINE_GLOBAL_INIT(T_gbh_ConvertColour,   pgbh_ConvertColour,   reinterpret_cast<T_gbh_ConvertColour>(GBH_Graphics_Stub_5EA4D0),   0x626C8C);
DEFINE_GLOBAL_INIT(T_gbh_RegisterTexture, pgbh_RegisterTexture, reinterpret_cast<T_gbh_RegisterTexture>(GBH_Graphics_Stub_5EA4D0), 0x626C90);
DEFINE_GLOBAL_INIT(T_gbh_SetCamera,       pgbh_SetCamera,       reinterpret_cast<T_gbh_SetCamera>(GBH_Graphics_Stub_5EA4D0),       0x626C94);
DEFINE_GLOBAL_INIT(T_gbh_ResetLights,     pgbh_ResetLights,     static_cast<T_gbh_ResetLights>(GBH_Graphics_Stub_5EA4D0),     0x626C98);
DEFINE_GLOBAL_INIT(T_gbh_AddLight,        pgbh_AddLight,        reinterpret_cast<T_gbh_AddLight>(GBH_Graphics_Stub_5EA4D0),        0x626C9C);
DEFINE_GLOBAL_INIT(T_gbh_SetAmbient,      pgbh_SetAmbient,      reinterpret_cast<T_gbh_SetAmbient>(GBH_Graphics_Stub_5EA4D0),      0x626CA0);
DEFINE_GLOBAL_INIT(T_gbh_GetUsedCache,    pgbh_GetUsedCache,    reinterpret_cast<T_gbh_GetUsedCache>(GBH_Graphics_Stub_5EA4D0),    0x626CA4);
DEFINE_GLOBAL_INIT(T_gbh_FreeTexture,     pgbh_FreeTexture,     reinterpret_cast<T_gbh_FreeTexture>(GBH_Graphics_Stub_5EA4D0),     0x626CA8);
DEFINE_GLOBAL_INIT(T_gbh_RegisterPalette, pgbh_RegisterPalette, reinterpret_cast<T_gbh_RegisterPalette>(GBH_Graphics_Stub_5EA4D0), 0x626CAC);
DEFINE_GLOBAL_INIT(T_gbh_FreePalette,     pgbh_FreePalette,     reinterpret_cast<T_gbh_FreePalette>(GBH_Graphics_Stub_5EA4D0),     0x626CB0);
DEFINE_GLOBAL_INIT(T_gbh_AssignPalette,   pgbh_AssignPalette,   reinterpret_cast<T_gbh_AssignPalette>(GBH_Graphics_Stub_5EA4D0),   0x626CB4);
DEFINE_GLOBAL_INIT(T_gbh_LockTexture,     pgbh_LockTexture,     reinterpret_cast<T_gbh_LockTexture>(GBH_Graphics_Stub_5EA4D0),     0x626CB8);
DEFINE_GLOBAL_INIT(T_gbh_UnlockTexture,   pgbh_UnlockTexture,   reinterpret_cast<T_gbh_UnlockTexture>(GBH_Graphics_Stub_5EA4D0),   0x626CBC);
DEFINE_GLOBAL_INIT(T_gbh_BeginScene,      pgbh_BeginScene,      static_cast<T_gbh_BeginScene>(GBH_Graphics_Stub_5EA4D0),      0x626CC0);
DEFINE_GLOBAL_INIT(T_gbh_EndScene,        pgbh_EndScene,        static_cast<T_gbh_EndScene>(GBH_Graphics_Stub_5EA4D0),        0x626CC4);
DEFINE_GLOBAL_INIT(T_gbh_BeginLevel,      pgbh_BeginLevel,      static_cast<T_gbh_BeginLevel>(GBH_Graphics_Stub_5EA4D0),      0x626CC8);
DEFINE_GLOBAL_INIT(T_gbh_EndLevel,        pgbh_EndLevel,        static_cast<T_gbh_EndLevel>(GBH_Graphics_Stub_5EA4D0),        0x626CCC);
DEFINE_GLOBAL_INIT(T_MakeScreenTable,     pMakeScreenTable,     reinterpret_cast<T_MakeScreenTable>(GBH_Graphics_Stub_5EA4D0),     0x626CD0);
DEFINE_GLOBAL_INIT(T_ConvertColourBank,   pConvertColourBank,   reinterpret_cast<T_ConvertColourBank>(GBH_Graphics_Stub_5EA4D0),   0x626CD4);
DEFINE_GLOBAL_INIT(T_SetShadeTableA,      pSetShadeTableA,      reinterpret_cast<T_SetShadeTableA>(GBH_Graphics_Stub_5EA4D0),      0x626CD8);
DEFINE_GLOBAL_INIT(T_gbh_InitImageTable,  pgbh_InitImageTable,  reinterpret_cast<T_gbh_InitImageTable>(GBH_Graphics_Stub_5EA4D0),  0x626CDC);
DEFINE_GLOBAL_INIT(T_gbh_FreeImageTable,  pgbh_FreeImageTable,  reinterpret_cast<T_gbh_FreeImageTable>(GBH_Graphics_Stub_5EA4D0),  0x626CE0);
DEFINE_GLOBAL_INIT(T_gbh_LoadImage,       pgbh_LoadImage,       reinterpret_cast<T_gbh_LoadImage>(GBH_Graphics_Stub_5EA4D0),       0x626CE4);
DEFINE_GLOBAL_INIT(T_gbh_BlitImage,       pgbh_BlitImage,       reinterpret_cast<T_gbh_BlitImage>(GBH_Graphics_Stub_5EA4D0),       0x626CE8);
DEFINE_GLOBAL_INIT(T_gbh_BlitBuffer,      pgbh_BlitBuffer,      reinterpret_cast<T_gbh_BlitBuffer>(GBH_Graphics_Stub_5EA4D0),      0x626CEC);
DEFINE_GLOBAL(HMODULE, gGbhDllHandle_7085D8, 0x7085D8);
DEFINE_GLOBAL(FARPROC, pDrawLine, 0x7085DC);
DEFINE_GLOBAL(s32, gGBH_GraphicsLoaded_7085E0, 0x7085E0);
DEFINE_GLOBAL(u32*, gpGBH_Globals_7085E4, 0x7085E4);

MATCH_FUNC(0x5EA4D0)
EXPORT void __stdcall GBH_Graphics_Stub_5EA4D0()
{
    MessageBoxA(NULL,
                "Error: A GBH Graphic function was called without the DLL being loaded, or the function has not been fixed up.",
                "GBH Graphics Error",
                MB_OK);
}

MATCH_FUNC(0x5EA500)
EXPORT void __stdcall GBH_Graphics_Unload_5EA500()
{
    FreeLibrary(gGbhDllHandle_7085D8);
    pgbh_InitDLL = reinterpret_cast<T_gbh_InitDLL>(GBH_Graphics_Stub_5EA4D0);
    pgbh_CloseDLL = static_cast<T_gbh_CloseDLL>(GBH_Graphics_Stub_5EA4D0);
    pgbh_Init = reinterpret_cast<T_gbh_Init>(GBH_Graphics_Stub_5EA4D0);
    pgbh_DrawTile = reinterpret_cast<T_gbh_DrawTile>(GBH_Graphics_Stub_5EA4D0);
    pgbh_DrawQuad = reinterpret_cast<T_gbh_DrawQuad>(GBH_Graphics_Stub_5EA4D0);
    pgbh_DrawQuadClipped = reinterpret_cast<T_gbh_DrawQuadClipped>(GBH_Graphics_Stub_5EA4D0);
    pgbh_DrawTriangle = reinterpret_cast<T_gbh_DrawTriangle>(GBH_Graphics_Stub_5EA4D0);
    pgbh_Plot = reinterpret_cast<T_gbh_Plot>(GBH_Graphics_Stub_5EA4D0);
    pgbh_SetWindow = reinterpret_cast<T_gbh_SetWindow>(GBH_Graphics_Stub_5EA4D0);
    pgbh_PrintBitmap = reinterpret_cast<T_gbh_PrintBitmap>(GBH_Graphics_Stub_5EA4D0);
    pgbh_SetColourDepth = reinterpret_cast<T_gbh_SetColourDepth>(GBH_Graphics_Stub_5EA4D0);
    pgbh_GetGlobals = reinterpret_cast<T_gbh_GetGlobals>(GBH_Graphics_Stub_5EA4D0);
    pgbh_ConvertColour = reinterpret_cast<T_gbh_ConvertColour>(GBH_Graphics_Stub_5EA4D0);
    pgbh_RegisterTexture = reinterpret_cast<T_gbh_RegisterTexture>(GBH_Graphics_Stub_5EA4D0);
    pConvertColourBank = reinterpret_cast<T_ConvertColourBank>(GBH_Graphics_Stub_5EA4D0);
    pDrawLine = reinterpret_cast<FARPROC>(GBH_Graphics_Stub_5EA4D0);
    pMakeScreenTable = reinterpret_cast<T_MakeScreenTable>(GBH_Graphics_Stub_5EA4D0);
    pSetShadeTableA = reinterpret_cast<T_SetShadeTableA>(GBH_Graphics_Stub_5EA4D0);
    pgbh_GetUsedCache = reinterpret_cast<T_gbh_GetUsedCache>(GBH_Graphics_Stub_5EA4D0);
    pgbh_SetCamera = reinterpret_cast<T_gbh_SetCamera>(GBH_Graphics_Stub_5EA4D0);
    pgbh_ResetLights = static_cast<T_gbh_ResetLights>(GBH_Graphics_Stub_5EA4D0);
    pgbh_AddLight = reinterpret_cast<T_gbh_AddLight>(GBH_Graphics_Stub_5EA4D0);
    pgbh_SetAmbient = reinterpret_cast<T_gbh_SetAmbient>(GBH_Graphics_Stub_5EA4D0);
    pgbh_InitImageTable = reinterpret_cast<T_gbh_InitImageTable>(GBH_Graphics_Stub_5EA4D0);
    pgbh_FreeImageTable = reinterpret_cast<T_gbh_FreeImageTable>(GBH_Graphics_Stub_5EA4D0);
    pgbh_LoadImage = reinterpret_cast<T_gbh_LoadImage>(GBH_Graphics_Stub_5EA4D0);
    pgbh_BlitImage = reinterpret_cast<T_gbh_BlitImage>(GBH_Graphics_Stub_5EA4D0);
    pgbh_BlitBuffer = reinterpret_cast<T_gbh_BlitBuffer>(GBH_Graphics_Stub_5EA4D0);
    pgbh_DrawFlatRect = reinterpret_cast<T_gbh_DrawFlatRect>(GBH_Graphics_Stub_5EA4D0);
}

MATCH_FUNC(0x5EA640)
EXPORT s32 GBH_Graphics_Free_5EA640()
{
    if (!gGBH_GraphicsLoaded_7085E0)
    {
        return 0;
    }

    gGBH_GraphicsLoaded_7085E0 = 0;
    pgbh_CloseDLL();
    GBH_Graphics_Unload_5EA500();
    return 0;
}

#define load_gbh_func(varName, varType, funcName)                      \
    varName = reinterpret_cast<varType>(GetProcAddress(gGbhDllHandle_7085D8, funcName)); \
    if (!varName)                                                      \
    {                                                                  \
        char_type msg[120];                                            \
        sprintf(msg, "Can't Find Function Called %s", funcName);       \
        MessageBoxA(NULL, msg, "Error Patching DLL Function", MB_OK);    \
        FreeLibrary(gGbhDllHandle_7085D8);                             \
        return -1;                                                     \
    }

MATCH_FUNC(0x5EA680)
EXPORT s32 __stdcall GBH_GraphicsLoadDll_5EA680(const char_type* lpLibFileName)
{
    char_type msg[126];
    if (!lpLibFileName)
    {
        return 0;
    }

    gGbhDllHandle_7085D8 = LoadLibraryA(lpLibFileName);
    if (gGbhDllHandle_7085D8)
    {
        load_gbh_func(pgbh_InitDLL, T_gbh_InitDLL, "gbh_InitDLL");
        load_gbh_func(pgbh_CloseDLL, T_gbh_CloseDLL, "gbh_CloseDLL");
        load_gbh_func(pgbh_Init, T_gbh_Init, "gbh_Init");
        load_gbh_func(pgbh_DrawTile, T_gbh_DrawTile, "gbh_DrawTile");
        load_gbh_func(pgbh_DrawTilePart, T_gbh_DrawTilePart, "gbh_DrawTilePart");
        load_gbh_func(pgbh_DrawQuad, T_gbh_DrawQuad, "gbh_DrawQuad");
        load_gbh_func(pgbh_DrawQuadClipped, T_gbh_DrawQuadClipped, "gbh_DrawQuadClipped");
        load_gbh_func(pgbh_DrawTriangle, T_gbh_DrawTriangle, "gbh_DrawTriangle");
        load_gbh_func(pgbh_Plot, T_gbh_Plot, "gbh_Plot");
        load_gbh_func(pgbh_SetWindow, T_gbh_SetWindow, "gbh_SetWindow");
        load_gbh_func(pgbh_PrintBitmap, T_gbh_PrintBitmap, "gbh_PrintBitmap");
        load_gbh_func(pgbh_SetColourDepth, T_gbh_SetColourDepth, "gbh_SetColourDepth");
        load_gbh_func(pgbh_GetGlobals, T_gbh_GetGlobals, "gbh_GetGlobals");
        load_gbh_func(pgbh_ConvertColour, T_gbh_ConvertColour, "gbh_ConvertColour");
        load_gbh_func(pgbh_RegisterTexture, T_gbh_RegisterTexture, "gbh_RegisterTexture");
        load_gbh_func(pgbh_BeginScene, T_gbh_BeginScene, "gbh_BeginScene");
        load_gbh_func(pgbh_EndScene, T_gbh_EndScene, "gbh_EndScene");
        load_gbh_func(pgbh_BeginLevel, T_gbh_BeginLevel, "gbh_BeginLevel");
        load_gbh_func(pgbh_EndLevel, T_gbh_EndLevel, "gbh_EndLevel");
        load_gbh_func(pConvertColourBank, T_ConvertColourBank, "ConvertColourBank");
        load_gbh_func(pDrawLine, FARPROC, "DrawLine");
        load_gbh_func(pMakeScreenTable, T_MakeScreenTable, "MakeScreenTable");
        load_gbh_func(pSetShadeTableA, T_SetShadeTableA, "SetShadeTableA");
        load_gbh_func(pgbh_UnlockTexture, T_gbh_UnlockTexture, "gbh_UnlockTexture");
        load_gbh_func(pgbh_RegisterPalette, T_gbh_RegisterPalette, "gbh_RegisterPalette");
        load_gbh_func(pgbh_FreePalette, T_gbh_FreePalette, "gbh_FreePalette");
        load_gbh_func(pgbh_FreeTexture, T_gbh_FreeTexture, "gbh_FreeTexture");
        load_gbh_func(pgbh_AssignPalette, T_gbh_AssignPalette, "gbh_AssignPalette");
        load_gbh_func(pgbh_LockTexture, T_gbh_LockTexture, "gbh_LockTexture");
        load_gbh_func(pgbh_GetUsedCache, T_gbh_GetUsedCache, "gbh_GetUsedCache");
        load_gbh_func(pgbh_SetCamera, T_gbh_SetCamera, "gbh_SetCamera");
        load_gbh_func(pgbh_ResetLights, T_gbh_ResetLights, "gbh_ResetLights");
        load_gbh_func(pgbh_AddLight, T_gbh_AddLight, "gbh_AddLight");
        load_gbh_func(pgbh_SetAmbient, T_gbh_SetAmbient, "gbh_SetAmbient");
        load_gbh_func(pgbh_InitImageTable, T_gbh_InitImageTable, "gbh_InitImageTable");
        load_gbh_func(pgbh_FreeImageTable, T_gbh_FreeImageTable, "gbh_FreeImageTable");
        load_gbh_func(pgbh_LoadImage, T_gbh_LoadImage, "gbh_LoadImage");
        load_gbh_func(pgbh_BlitImage, T_gbh_BlitImage, "gbh_BlitImage");
        load_gbh_func(pgbh_BlitBuffer, T_gbh_BlitBuffer, "gbh_BlitBuffer");
        load_gbh_func(pgbh_DrawFlatRect, T_gbh_DrawFlatRect, "gbh_DrawFlatRect");
    }
    else
    {
        sprintf(msg, "Error can't load DLL file: %s", lpLibFileName);
        MessageBoxA(NULL, msg, "Error Loading DLL.", MB_OK);
        return -1;
    }
    return 0;
}

MATCH_FUNC(0x5EB680)
EXPORT s32 __stdcall GBH_GraphicsLoad_5EB680(const char_type* lpLibFileName, SVideo* pVidSys)
{
    const s32 error = GBH_GraphicsLoadDll_5EA680(lpLibFileName);
    if (error)
    {
        return error;
    }

    pgbh_InitDLL(pVidSys);
    gpGBH_Globals_7085E4 = pgbh_GetGlobals();

    gGBH_GraphicsLoaded_7085E0 = 1;

    return 0;
}

#endif // GTA2_MODERN_BUILD