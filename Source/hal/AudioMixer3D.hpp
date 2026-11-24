#pragma once

#include "HalTypes.hpp"
#include <cmath>
#include <algorithm>

namespace gta2::hal {

struct ListenerState {
    f32 posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    f32 dirX = 0.0f, dirY = 1.0f, dirZ = 0.0f; // Forward vector
    f32 upX  = 0.0f, upY  = 0.0f, upZ  = 1.0f; // Up vector
};

class AudioMixer3D {
public:
    static void CalculateSpatial3D(
        const ListenerState& listener,
        f32 emitterX, f32 emitterY, f32 emitterZ,
        f32 minDistance, f32 maxDistance,
        f32 baseVolume,
        f32& outVolume,
        f32& outPan
    ) {
        f32 dx = emitterX - listener.posX;
        f32 dy = emitterY - listener.posY;
        f32 dz = emitterZ - listener.posZ;

        f32 dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        // Distance Attenuation
        if (dist <= minDistance) {
            outVolume = baseVolume;
        } else if (dist >= maxDistance) {
            outVolume = 0.0f;
        } else {
            f32 factor = (dist - minDistance) / (maxDistance - minDistance);
            outVolume = baseVolume * (1.0f - factor);
        }

        // Stereo Panning (X-axis relative to listener)
        f32 pan = (maxDistance > 0.0f) ? (dx / maxDistance) : 0.0f;
        outPan = std::clamp(pan, -1.0f, 1.0f);
    }
};

} // namespace gta2::hal