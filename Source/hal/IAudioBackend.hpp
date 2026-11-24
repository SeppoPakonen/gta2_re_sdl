#pragma once
#include "HalTypes.hpp"

namespace gta2::hal {

using SoundId = s32;
constexpr SoundId kInvalidSoundId = -1;

using ChannelHandle = s32;
constexpr ChannelHandle kInvalidChannelHandle = -1;

struct SoundParams3D {
    f32 posX, posY, posZ;
    f32 velX, velY, velZ;
    f32 minDistance;
    f32 maxDistance;
};

class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    virtual bool Initialize(s32 sampleRate, s32 channels, s32 bufferSize) = 0;
    virtual void Shutdown() = 0;

    // Sample Management
    virtual SoundId LoadSampleFromMemory(const void* pPcmData, size_t dataSize, s32 sampleRate, s32 bits, s32 channels) = 0;
    virtual void UnloadSample(SoundId soundId) = 0;

    // Playback
    virtual ChannelHandle PlaySample(SoundId soundId, f32 volume, f32 pan, bool loop) = 0;
    virtual ChannelHandle PlaySample3D(SoundId soundId, f32 volume, const SoundParams3D& params, bool loop) = 0;
    virtual void StopChannel(ChannelHandle channel) = 0;
    virtual void SetChannelVolume(ChannelHandle channel, f32 volume) = 0;
    virtual void SetChannelPan(ChannelHandle channel, f32 pan) = 0;
    virtual void SetChannelPitch(ChannelHandle channel, f32 pitch) = 0;
    virtual void UpdateListener3D(f32 posX, f32 posY, f32 posZ, f32 dirX, f32 dirY, f32 dirZ) = 0;

    // Master Controls
    virtual void SetMasterVolume(f32 volume) = 0;
    virtual void StopAllChannels() = 0;
    virtual void Update() = 0;
};

} // namespace gta2::hal