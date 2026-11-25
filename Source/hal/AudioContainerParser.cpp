#include "AudioContainerParser.hpp"
#include "HalTypes.hpp"
#include "HalContext.hpp"
#include "VfsFileSystem.hpp"

#include <algorithm>
#include <cstring>

namespace gta2::hal {

bool AudioContainerParser::LoadContainer(
    const std::string& sdtPath,
    const std::string& rawPath,
    std::vector<LoadedSoundSample>& outSamples
)
{
    IFileSystem* fs = GetHal().pFileSystem;
    if (!fs) {
        return false;
    }

    // ---------- Read the SDT (Sample Descriptor Table) ----------
    std::vector<AudioSampleDescriptor> entries;
    {
        IFileStream* sdtStream = fs->OpenFile(sdtPath.c_str(), "rb");
        if (!sdtStream) {
            return false;
        }

        s64 sdtSize = sdtStream->GetSize();
        // Each SdtEntry is sizeof(AudioSampleDescriptor) == 24 bytes (0x18).
        s64 entryCount = sdtSize / sizeof(AudioSampleDescriptor);
        std::vector<u8> sdtData(sdtSize);
        if (sdtSize > 0) {
            s64 read = sdtStream->Read(sdtData.data(), sdtSize);
            (void)read; // should equal sdtSize
        }

        for (s64 i = 0; i < entryCount; ++i) {
            const AudioSampleDescriptor* desc
                = reinterpret_cast<const AudioSampleDescriptor*>(
                    sdtData.data() + i * sizeof(AudioSampleDescriptor));
            entries.push_back(*desc);
        }

        sdtStream->Close();
        delete sdtStream;
    }

    // ---------- Read the raw PCM data ----------
    {
        IFileStream* rawStream = fs->OpenFile(rawPath.c_str(), "rb");
        if (!rawStream) {
            // Free already-read entries? For simplicity just return.
            return false;
        }

        s64 rawSize = rawStream->GetSize();
        std::vector<u8> rawData(rawSize);
        if (rawSize > 0) {
            rawStream->Read(rawData.data(), rawSize);
        }

        rawStream->Close();
        delete rawStream;

        // Slice the raw buffer into individual samples.
        for (const auto& desc : entries) {
            LoadedSoundSample sample;
            sample.desc = desc;
            sample.channels = 1; // GTA2 .raw files are 16-bit mono PCM
            sample.bitsPerSample = 16;

            s32 off = desc.offset;
            s32 len = desc.sampleLength;
            // Clamp to available data.
            if (off < 0) off = 0;
            if (len < 0) len = 0;
            s32 avail = static_cast<s32>(rawData.size()) - off;
            if (avail < 0) avail = 0;
            s32 toRead = std::min(len, avail);

            sample.pcmData.assign(rawData.begin() + off, rawData.begin() + off + toRead);
            outSamples.push_back(sample);
        }
    }

    return true;
}

} // namespace gta2::hal