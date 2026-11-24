#include "SDLAudioBackend.hpp"

#if defined(GTA2_USE_SDL3)
    // SDL3 implementation would go here (audio streams, SDL_CreateAudioStream, etc.)
#else
    #include <SDL2/SDL.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstring>

namespace gta2::hal {

static void SDLCALL AudioCallback(void* userdata, Uint8* stream, int len) {
    SDLAudioBackend* self = static_cast<SDLAudioBackend*>(userdata);
    if (self) {
        self->MixInto(stream, len);
    }
}

bool SDLAudioBackend::Initialize(s32 sampleRate, s32 channels, s32 bufferSize) {
    if (mInitialized) {
        return true;
    }

    mChannels.resize(MaxChannels);

#if defined(GTA2_USE_SDL3)
    // SDL3 initialization path omitted (guarded out by GTA2_USE_SDL3)
    return false;
#else
    SDL_AudioSpec spec{};
    spec.freq = sampleRate;
    spec.format = AUDIO_S16SYS;
    spec.channels = static_cast<Uint8>(channels);
    spec.samples = static_cast<Uint16>(bufferSize);
    spec.callback = AudioCallback;
    spec.userdata = this;

    mAudioDevice = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (mAudioDevice == 0) {
        return false;
    }

    SDL_PauseAudioDevice(mAudioDevice, 0);
    mInitialized = true;
    mMasterVolume = 1.0f;
    return true;
#endif
}

void SDLAudioBackend::Shutdown() {
    if (!mInitialized) {
        return;
    }

#if defined(GTA2_USE_SDL3)
    // SDL3 shutdown omitted
#else
    if (mAudioDevice != 0) {
        SDL_PauseAudioDevice(mAudioDevice, 1);
        SDL_CloseAudioDevice(mAudioDevice);
        mAudioDevice = 0;
    }
#endif

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mChannels.clear();
        mSamples.clear();
    }
    mInitialized = false;
}

SoundId SDLAudioBackend::LoadSampleFromMemory(const void* pPcmData, size_t dataSize,
                                              s32 sampleRate, s32 bits, s32 channels) {
    if (!pPcmData || dataSize == 0) {
        return kInvalidSoundId;
    }

    LoadedSampleData data;
    data.pcm.assign(static_cast<const u8*>(pPcmData),
                    static_cast<const u8*>(pPcmData) + dataSize);
    data.sampleRate = sampleRate;
    data.bitsPerSample = bits;
    data.channels = channels;
    if (bits == 16 && channels > 0) {
        data.frames = dataSize / (static_cast<size_t>(channels) * (bits / 8));
    }

    std::lock_guard<std::mutex> lock(mMutex);
    SoundId id = mNextSoundId++;
    mSamples.emplace(id, std::move(data));
    return id;
}

void SDLAudioBackend::UnloadSample(SoundId soundId) {
    std::lock_guard<std::mutex> lock(mMutex);
    mSamples.erase(soundId);
    // Stop any channels using this sound
    for (auto& ch : mChannels) {
        if (ch.active && ch.soundId == soundId) {
            ch.active = false;
        }
    }
}

ChannelHandle SDLAudioBackend::PlaySample(SoundId soundId, f32 volume, f32 pan, bool loop) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (mSamples.find(soundId) == mSamples.end()) {
        return kInvalidChannelHandle;
    }

    for (size_t i = 0; i < mChannels.size(); ++i) {
        VoiceChannel& ch = mChannels[i];
        if (!ch.active) {
            ch.active = true;
            ch.soundId = soundId;
            ch.cursor = 0;
            ch.volume = std::clamp(volume, 0.0f, 1.0f);
            ch.pan = std::clamp(pan, -1.0f, 1.0f);
            ch.loop = loop;
            return static_cast<ChannelHandle>(i);
        }
    }
    return kInvalidChannelHandle;
}

ChannelHandle SDLAudioBackend::PlaySample3D(SoundId soundId, f32 volume,
                                            const SoundParams3D& params, bool loop) {
    // 3D playback: for now treat as regular PlaySample with the given volume.
    // A full 3D implementation would apply distance attenuation, Doppler, etc.
    (void)params;
    return PlaySample(soundId, volume, 0.0f, loop);
}

void SDLAudioBackend::StopChannel(ChannelHandle channel) {
    std::lock_guard<std::mutex> lock(mMutex);
    if (channel >= 0 && static_cast<size_t>(channel) < mChannels.size()) {
        mChannels[static_cast<size_t>(channel)].active = false;
    }
}

void SDLAudioBackend::SetChannelVolume(ChannelHandle channel, f32 volume) {
    std::lock_guard<std::mutex> lock(mMutex);
    if (channel >= 0 && static_cast<size_t>(channel) < mChannels.size()) {
        mChannels[static_cast<size_t>(channel)].volume = std::clamp(volume, 0.0f, 1.0f);
    }
}

void SDLAudioBackend::SetChannelPan(ChannelHandle channel, f32 pan) {
    std::lock_guard<std::mutex> lock(mMutex);
    if (channel >= 0 && static_cast<size_t>(channel) < mChannels.size()) {
        mChannels[static_cast<size_t>(channel)].pan = std::clamp(pan, -1.0f, 1.0f);
    }
}

void SDLAudioBackend::SetChannelPitch(ChannelHandle channel, f32 pitch) {
    // Pitch change would require resampling; not implemented in this simple mixer.
    (void)channel;
    (void)pitch;
}

void SDLAudioBackend::UpdateListener3D(f32 posX, f32 posY, f32 posZ,
                                       f32 dirX, f32 dirY, f32 dirZ) {
    (void)posX; (void)posY; (void)posZ;
    (void)dirX; (void)dirY; (void)dirZ;
    // No-op for stereo panning backend
}

void SDLAudioBackend::SetMasterVolume(f32 volume) {
    std::lock_guard<std::mutex> lock(mMutex);
    mMasterVolume = std::clamp(volume, 0.0f, 1.0f);
}

void SDLAudioBackend::StopAllChannels() {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto& ch : mChannels) {
        ch.active = false;
    }
}

void SDLAudioBackend::Update() {
    // In callback-driven mode, no periodic work is needed.
    // Could be used for 3D position updates or automatic cleanup.
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto& ch : mChannels) {
        if (ch.active) {
            auto it = mSamples.find(ch.soundId);
            if (it != mSamples.end() && !ch.loop) {
                if (ch.cursor >= it->second.frames) {
                    ch.active = false;
                }
            }
        }
    }
}

void SDLAudioBackend::MixInto(uint8_t* stream, int len) {
    std::lock_guard<std::mutex> lock(mMutex);

    const size_t totalFrames = static_cast<size_t>(len) / 2; // 16-bit = 2 bytes per sample (per channel)
    if (totalFrames == 0) return;

    // Zero-initialize the output buffer
    std::memset(stream, 0, len);

    for (VoiceChannel& ch : mChannels) {
        if (!ch.active) continue;

        auto it = mSamples.find(ch.soundId);
        if (it == mSamples.end()) {
            ch.active = false;
            continue;
        }
        const LoadedSampleData& sample = it->second;
        if (sample.pcm.empty() || sample.frames == 0) {
            ch.active = false;
            continue;
        }

        const size_t frameSize = static_cast<size_t>(sample.channels) * 2; // 16-bit = 2 bytes
        const size_t framesAvail = (sample.frames > ch.cursor) ? (sample.frames - ch.cursor) : 0;
        if (framesAvail == 0) {
            if (ch.loop) {
                ch.cursor = 0;
            } else {
                ch.active = false;
                continue;
            }
        }

        const size_t framesToMix = std::min(framesAvail, totalFrames);
        const f32 vol = ch.volume * mMasterVolume;
        const f32 pan = ch.pan;

        // Per-channel pan gains: pan=-1 => left=1/right=0; pan=0 => both=1; pan=+1 => left=0/right=1
        const f32 leftGain  = (pan < 0.0f) ? (1.0f + pan) : (1.0f - pan);
        const f32 rightGain = (pan > 0.0f) ? (1.0f + pan) : (1.0f - pan);

        const int16_t* src = reinterpret_cast<const int16_t*>(sample.pcm.data());
        int16_t* dst = reinterpret_cast<int16_t*>(stream);

        for (size_t i = 0; i < framesToMix; ++i) {
            size_t srcIdx = (ch.cursor + i) * sample.channels;
            int16_t leftSample, rightSample;
            if (sample.channels == 1) {
                // Mono: duplicate to both channels
                int16_t s = src[srcIdx];
                leftSample = s;
                rightSample = s;
            } else if (sample.channels >= 2) {
                leftSample = src[srcIdx];
                rightSample = src[srcIdx + 1];
            } else {
                continue;
            }

            int32_t mixedL = static_cast<int32_t>(leftSample) * vol * leftGain;
            int32_t mixedR = static_cast<int32_t>(rightSample) * vol * rightGain;

            size_t dstIdx = i * 2; // output is always stereo
            // Additive mix with saturation
            mixedL = std::clamp(mixedL + dst[dstIdx], -32768, 32767);
            mixedR = std::clamp(mixedR + dst[dstIdx + 1], -32768, 32767);

            dst[dstIdx] = static_cast<int16_t>(mixedL);
            dst[dstIdx + 1] = static_cast<int16_t>(mixedR);
        }

        ch.cursor += framesToMix;
        if (!ch.loop && ch.cursor >= sample.frames) {
            ch.active = false;
        }
    }
}

} // namespace gta2::hal