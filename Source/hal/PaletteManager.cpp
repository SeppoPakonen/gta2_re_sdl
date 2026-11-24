#include "PaletteManager.hpp"

namespace gta2::hal {

const std::vector<u32>& PaletteManager::Convert(const u8* pSrcPixels, const u16* pPalette,
                                                size_t pixelCount, bool hasAlpha) {
    const PaletteCacheKey key{pSrcPixels, pPalette, pixelCount, hasAlpha};

    auto it = mCache.find(key);
    if (it != mCache.end()) {
        return it->second;
    }

    std::vector<u32> converted(pixelCount);
    Expand8BitTo32Bit(pSrcPixels, pPalette, converted.data(), pixelCount, hasAlpha);

    auto inserted = mCache.emplace(key, std::move(converted));
    return inserted.first->second;
}

void PaletteManager::ClearCache() {
    mCache.clear();
}

size_t PaletteManager::CacheSize() const {
    return mCache.size();
}

} // namespace gta2::hal