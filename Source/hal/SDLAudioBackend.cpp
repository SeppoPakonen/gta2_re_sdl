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
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        return false;
    }

    SDL_AudioSpec spec{};
    spec.freq = sampleRate;
    spec.format = AUDIO_S16SYS;
    spec.channels = static_cast<Uint8>(channels);
    spec.samples = static_cast<Uint16>(bufferSize);
    spec.callback = AudioCallback;
    spec.userdata = this;

    SDL_AudioSpec obtained{};
    mAudioDevice = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, 0);
    if (mAudioDevice == 0) {
        printf("[AUDIO-INIT] SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return false;
    }

    printf("[AUDIO-INIT] SDL Audio initialized successfully: device=%u, freq=%d, channels=%d, samples=%d\n",
           mAudioDevice, obtained.freq, obtained.channels, obtained.samples);
    SDL_PauseAudioDevice(mAudioDevice, 0);
    mInitialized = true;
    mMasterVolume = 1.0f;
    mDeviceSampleRate = 22050;
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
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
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
            ch.pitch = 1.0f;
            ch.sampleRate = 44100.0f;
            printf("[AUDIO] PlaySample: soundId=%u vol=%.2f pan=%.2f loop=%d -> voice channel %zu\n",
                   soundId, volume, pan, loop ? 1 : 0, i);
            return static_cast<ChannelHandle>(i);
        }
    }
    return kInvalidChannelHandle;
}

ChannelHandle SDLAudioBackend::PlaySample3D(SoundId soundId, f32 volume,
                                            const SoundParams3D& params, bool loop) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (mSamples.find(soundId) == mSamples.end()) {
        return kInvalidChannelHandle;
    }

    // Compute 3D audio parameters from sound position relative to listener.
    // Distance attenuation (linear between minDistance and maxDistance).
    f32 dx = params.posX - mListenerPos.x;
    f32 dy = params.posY - mListenerPos.y;
    f32 dz = params.posZ - mListenerPos.z;
    f32 distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    f32 attenuation = 1.0f;
    if (params.maxDistance > params.minDistance) {
        if (distance <= params.minDistance) {
            attenuation = 1.0f;
        } else if (distance >= params.maxDistance) {
            attenuation = 0.0f;
        } else {
            attenuation = 1.0f - (distance - params.minDistance) / (params.maxDistance - params.minDistance);
        }
    }

    // Compute pan based on angle relative to listener forward direction.
    // Project sound position onto listener's horizontal plane (X-Z).
    f32 sx = dx / (distance + 1e-4f);
    f32 sz = dz / (distance + 1e-4f);
    f32 pan = std::atan2(sx, sz) / 3.14159265f; // [-1, 1] approx

    // Doppler pitch shift from relative velocity along sound-to-listener line.
    f32 relVelX = params.velX;
    f32 relVelZ = params.velZ;
    f32 radialVel = (relVelX * sx + relVelZ * sz); // positive = approaching
    f32 pitch = 1.0f + radialVel / 340.0f; // speed of sound ~340 m/s

    // Find free channel
    for (size_t i = 0; i < mChannels.size(); ++i) {
        VoiceChannel& ch = mChannels[i];
        if (!ch.active) {
            ch.active = true;
            ch.soundId = soundId;
            ch.cursor = 0;
            ch.volume = std::clamp(volume * attenuation, 0.0f, 1.0f);
            ch.pan = std::clamp(pan, -1.0f, 1.0f);
            ch.loop = loop;
            ch.pitch = std::clamp(pitch, 0.5f, 2.0f);
            printf("[AUDIO] PlaySample3D: soundId=%u vol=%.2f pan=%.2f pitch=%.2f loop=%d -> voice channel %zu\n",
                   soundId, ch.volume, ch.pan, ch.pitch, loop ? 1 : 0, i);
            return static_cast<ChannelHandle>(i);
        }
    }
    return kInvalidChannelHandle;
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
    std::lock_guard<std::mutex> lock(mMutex);
    if (channel >= 0 && static_cast<size_t>(channel) < mChannels.size()) {
        mChannels[static_cast<size_t>(channel)].pitch = std::clamp(pitch, 0.0f, 4.0f);
    }
}

void SDLAudioBackend::UpdateListener3D(f32 posX, f32 posY, f32 posZ,
                                       f32 dirX, f32 dirY, f32 dirZ) {
    std::lock_guard<std::mutex> lock(mMutex);
    mListenerPos = {posX, posY, posZ};
    mListenerDir = {dirX, dirY, dirZ};
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

    const size_t totalFrames = static_cast<size_t>(len) / 4; // 16-bit stereo = 4 bytes per frame
    if (totalFrames == 0) return;

    // Zero-initialize the output buffer
    std::memset(stream, 0, len);
    int16_t* dst = reinterpret_cast<int16_t*>(stream);

    // Device sample rate (SDL2 default 22050)
    const s32 deviceRate = mDeviceSampleRate;

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

        // Per-channel resampling: source sampleRate -> device rate.
        // Playback speed factor = sourceRate / deviceRate.
        // A sourceRate > deviceRate => faster playback (higher pitch).
        // A sourceRate < deviceRate => slower playback (lower pitch).
        const f32 sourceRate = static_cast<f32>(sample.sampleRate);
        const f32 ratio = sourceRate / static_cast<f32>(deviceRate);
        const size_t framesAvail = static_cast<size_t>(sample.frames) - ch.cursor;
        if (framesAvail == 0) {
            if (ch.loop) {
                ch.cursor = 0;
            } else {
                ch.active = false;
                continue;
            }
        }

        const f32 vol = ch.volume * mMasterVolume;
        const f32 pan = ch.pan;

        // Per-channel pan gains: pan=-1 => left=1/right=0; pan=0 => both=1; pan=+1 => left=0/right=1
        const f32 leftGain  = (pan < 0.0f) ? (1.0f + pan) : (1.0f - pan);
        const f32 rightGain = (pan > 0.0f) ? (1.0f + pan) : (1.0f - pan);

        // Resample each output frame from source data, applying pitch if set.
        float srcIdxBase = static_cast<float>(ch.cursor);
        const f32 effectiveRatio = ratio * ch.pitch;
        for (size_t outFrame = 0; outFrame < totalFrames; ++outFrame) {
            // Source sample index for this output frame, accounting for rate ratio.
            float srcIdx = srcIdxBase + static_cast<float>(outFrame) * effectiveRatio;
            // Clamp to valid source range.
            if (srcIdx < 0.0f || srcIdx >= static_cast<float>(sample.frames)) break;
            int16_t baseIdx = static_cast<int>(srcIdx);

            int16_t leftSample, rightSample;
            if (sample.channels == 1) {
                // Mono: duplicate to both channels
                if (baseIdx < 0 || static_cast<size_t>(baseIdx) >= sample.pcm.size()) break;
                leftSample = rightSample = sample.pcm[baseIdx];
            } else if (sample.channels >= 2) {
                if (baseIdx < 0 || static_cast<size_t>(baseIdx) * 2 + 1 >= sample.pcm.size()) break;
                leftSample = sample.pcm[baseIdx * 2];
                rightSample = sample.pcm[baseIdx * 2 + 1];
            } else {
                continue;
            }

            // Apply per-channel pitch: if pitch != 1.0f, adjust cursor
            // such that effective playback speed changes.
            // We advance cursor by effectiveRatio per output frame below.

            // Additive mix with saturation and volume/pan
            int32_t mixedL = static_cast<int32_t>(leftSample) * vol * leftGain;
            int32_t mixedR = static_cast<int32_t>(rightSample) * vol * rightGain;

            size_t dstIdx = outFrame * 2; // output is always stereo
            // Additive mix with saturation
            mixedL = std::clamp(mixedL + dst[dstIdx], -32768, 32767);
            mixedR = std::clamp(mixedR + dst[dstIdx + 1], -32768, 32767);

            dst[dstIdx] = static_cast<int16_t>(mixedL);
            dst[dstIdx + 1] = static_cast<int16_t>(mixedR);
        }

        // Advance cursor by source frames consumed per output frame
        ch.cursor += static_cast<s32>(static_cast<float>(totalFrames) * effectiveRatio);
        // Clamp cursor to sample frames
        if (ch.cursor > static_cast<s32>(sample.frames)) {
            ch.cursor = static_cast<s32>(sample.frames);
        }

        // If not looping and we've consumed all source frames, stop channel
        if (!ch.loop && ch.cursor >= static_cast<s32>(sample.frames)) {
            ch.active = false;
        }
    }

    uint32_t nonZeroInThisBuffer = 0;
    for (int b = 0; b < len; ++b) {
        if (stream[b] != 0) {
            nonZeroInThisBuffer++;
        }
    }
    static uint64_t sTotalNonZeroBytes = 0;
    static uint32_t sMixCallbackCount = 0;
    sTotalNonZeroBytes += nonZeroInThisBuffer;
    sMixCallbackCount++;
    if (sMixCallbackCount % 100 == 0 || (nonZeroInThisBuffer > 0 && sMixCallbackCount <= 10)) {
        printf("[AUDIO-MIX] Callback #%u: %u/%d non-zero bytes mixed (total non-zero bytes sent: %llu)\n",
               sMixCallbackCount, nonZeroInThisBuffer, len, static_cast<unsigned long long>(sTotalNonZeroBytes));
        fflush(stdout);
    }
}

} // namespace gta2::hal