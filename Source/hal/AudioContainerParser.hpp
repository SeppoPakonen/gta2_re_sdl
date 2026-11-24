#pragma once

#include "HalTypes.hpp"
#include <vector>
#include <string>

namespace gta2::hal {

struct AudioSampleDescriptor {
    s32 offset;              // Byte offset into .raw file
    s32 sampleLength;        // Length in bytes
    s32 playbackRate;        // Typically 11025 or 22050 Hz
    s32 randomDisplacement;
    s32 loopStart;
    s32 loopEnd;             // -1 if non-looping
};

struct LoadedSoundSample {
    AudioSampleDescriptor desc;
    std::vector<u8> pcmData;
    s32 channels; // 1 (mono) or 2 (stereo)
    s32 bitsPerSample; // Typically 16-bit signed PCM
};

class AudioContainerParser {
public:
    static bool LoadContainer(
        const std::string& sdtPath,
        const std::string& rawPath,
        std::vector<LoadedSoundSample>& outSamples
    );
};

} // namespace gta2::hal