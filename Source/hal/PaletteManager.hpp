#pragma once

#include "HalTypes.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace gta2::hal {

// Convert a 16-bit GTA2 RGB565 colour to 32-bit RGBA (in memory: R, G, B, A).
inline u32 Convert16BitTo32Bit(u16 color16, bool /*hasAlphaBit*/) {
    // RGB 565
    const u8 a = 0xFF;
    const u8 r = static_cast<u8>(((color16 >> 11) & 0x1F) * 255 / 31);
    const u8 g = static_cast<u8>(((color16 >> 5) & 0x3F) * 255 / 63);
    const u8 b = static_cast<u8>((color16 & 0x1F) * 255 / 31);
    // On x86 little-endian, byte 0 is R, byte 1 is G, byte 2 is B, byte 3 is A
    return (static_cast<u32>(a) << 24) | (static_cast<u32>(b) << 16) |
           (static_cast<u32>(g) << 8) | static_cast<u32>(r);
}

// Expand an 8-bit paletted image to 32-bit RGBA. Index 0 is treated as the
// transparent colour key when hasAlpha is set.
inline void Expand8BitTo32Bit(const u8* pSrcPixels, const u16* pPalette, u32* pDstPixels,
                              size_t pixelCount, bool hasAlpha) {
    for (size_t i = 0; i < pixelCount; ++i) {
        const u8 index = pSrcPixels[i];
        if (index == 0 && hasAlpha) {
            pDstPixels[i] = 0x00000000; // Transparent key
        } else {
            pDstPixels[i] = Convert16BitTo32Bit(pPalette[index], false);
        }
    }
}

// Identifies a converted-texture cache entry: the source 8-bit pixels, the
// palette contents, the pixel count and whether the alpha key applies.
struct PaletteCacheKey {
    const void* pPixels;
    const void* pPalette;
    size_t pixelCount;
    bool hasAlpha;

    bool operator==(const PaletteCacheKey& other) const {
        return pPixels == other.pPixels && pPalette == other.pPalette &&
               pixelCount == other.pixelCount && hasAlpha == other.hasAlpha;
    }
};

struct PaletteCacheKeyHash {
    size_t operator()(const PaletteCacheKey& key) const {
        size_t h = std::hash<const void*>()(key.pPixels);
        h ^= std::hash<const void*>()(key.pPalette) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<size_t>()(key.pixelCount) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= static_cast<size_t>(key.hasAlpha) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

// Converts 8-bit indexed GTA2 sprites/tiles into 32-bit RGBA8888 buffers the
// GPU can sample directly, caching results so reused sprite+palette
// combinations avoid re-conversion (GTA2 swaps palettes per car/ped colour).
class PaletteManager {
public:
    // Returns a cached 32-bit RGBA buffer for the given 8-bit source + palette.
    // The returned reference stays valid until the cache is cleared or the
    // entry is evicted.
    const std::vector<u32>& Convert(const u8* pSrcPixels, const u16* pPalette,
                                    size_t pixelCount, bool hasAlpha);

    void ClearCache();
    size_t CacheSize() const;

private:
    std::unordered_map<PaletteCacheKey, std::vector<u32>, PaletteCacheKeyHash> mCache;
};

} // namespace gta2::hal