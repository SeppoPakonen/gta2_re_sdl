#pragma once

#include "IAudioBackend.hpp"
#include <vector>
#include <mutex>
#include <unordered_map>

#if defined(GTA2_USE_SDL3)
    #include <SDL3/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

namespace gta2::hal {

struct VoiceChannel {
    bool active = false;
    SoundId soundId = kInvalidSoundId;
    size_t cursor = 0;
    f32 volume = 1.0f;
    f32 pan = 0.0f; // -1.0 (left) to +1.0 (right)
    bool loop = false;
};

class SDLAudioBackend : public IAudioBackend {
private:
    struct LoadedSampleData {
        std::vector<u8> pcm;
        s32 sampleRate = 44100;
        s32 bitsPerSample = 16;
        s32 channels = 2;
        size_t frames = 0;
    };

    static int constexpr MaxChannels = 64;

    std::mutex mMutex;
    std::vector<VoiceChannel> mChannels;
    std::unordered_map<SoundId, LoadedSampleData> mSamples;
#if defined(GTA2_USE_SDL3)
    SDL_AudioStream* mAudioStream = nullptr;
#else
    SDL_AudioDeviceID mAudioDevice = 0;
#endif
    bool mInitialized = false;
    f32 mMasterVolume = 1.0f;
    SoundId mNextSoundId = 1;

    public:
        void MixInto(uint8_t* stream, int len);

public:
    bool Initialize(s32 sampleRate, s32 channels, s32 bufferSize) override;
    void Shutdown() override;

    SoundId LoadSampleFromMemory(const void* pPcmData, size_t dataSize, s32 sampleRate, s32 bits, s32 channels) override;
    void UnloadSample(SoundId soundId) override;

    ChannelHandle PlaySample(SoundId soundId, f32 volume, f32 pan, bool loop) override;
    ChannelHandle PlaySample3D(SoundId soundId, f32 volume, const SoundParams3D& params, bool loop) override;
    void StopChannel(ChannelHandle channel) override;
    void SetChannelVolume(ChannelHandle channel, f32 volume) override;
    void SetChannelPan(ChannelHandle channel, f32 pan) override;
    void SetChannelPitch(ChannelHandle channel, f32 pitch) override;

    void UpdateListener3D(f32 posX, f32 posY, f32 posZ, f32 dirX, f32 dirY, f32 dirZ) override;

    void SetMasterVolume(f32 volume) override;
    void StopAllChannels() override;
    void Update() override;
};

} // namespace gta2::hal
